import torch
import socket
import gym
import torch as th
import numpy as np
from collections import deque
from gym import spaces


from stable_baselines3 import DQN
from stable_baselines3.common.torch_layers import BaseFeaturesExtractor
from stable_baselines3.common.callbacks import CheckpointCallback, BaseCallback


from csp_py.main import ROOT_DIR

class RawFeature(BaseFeaturesExtractor):
    def __init__(self, observation_space, features_dim):
        super(RawFeature, self).__init__(observation_space, features_dim)

    def forward(self, observations: th.Tensor) -> th.Tensor:
        return observations


class CSPPEnvSocket(gym.Env):
    """
    c++ environment (socket based)
    """
    EXPAND_CHOICES = [16, 32, 64, 128, 256, 512, 1024]
    N_DISCRETE_ACTIONS = len(EXPAND_CHOICES) # 7
    N_OBSERVATIONS = 8

    def __init__(self, u, v, cost_limit, queries=None, multi_query_train=False):
        self.host = '127.0.0.1'  # The server's hostname or IP address
        self.port =  12345
        self.BUFFER_SIZE = 512
        self.connected = False

        self.action_space = spaces.Discrete(CSPPEnvSocket.N_DISCRETE_ACTIONS)
        self.observation_space = spaces.Box(low=0, high=1000000, shape=(CSPPEnvSocket.N_OBSERVATIONS,), dtype=int)


        self.queries = queries
        self.q_idx = 0
        self.multi_query_train = multi_query_train
        self.set_q(u, v, cost_limit) # default setting

        if multi_query_train is True:
            assert self.queries is not None

    
    def set_q(self, u, v, cost_limit):
        self.u = u
        self.v = v
        self.cost_limit = cost_limit


    def connect_socket(self):
        if not self.connected:
            self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            print('connecting')
            self.s.connect((self.host, self.port))
            print('connected')
            self.connected = True
    
    def close_socket(self):
        if self.connected:
            str_b = 'close'.encode('ascii')
            self.s.sendall(str_b)
            self.s.close()
            print('closed')
            self.connected = False
    
    def reset(self):
        """
        Standard function for gym environment
        reset should not accept params
        """

        if self.multi_query_train:
            u = self.queries[self.q_idx][0]
            v = self.queries[self.q_idx][1]
            cost_limit = self.queries[self.q_idx][2]
            self.set_q(u, v, cost_limit)
            self.q_idx = (self.q_idx+1) % len(self.queries)

        str_b = f'reset {self.u} {self.v} {self.cost_limit}'.encode('ascii')
        self.s.sendall(str_b)
        rec = self.s.recv(self.BUFFER_SIZE)
        rec = rec.decode('ascii')

        rec = rec.strip('reset ok ')
        rec = rec.split(' ')
        rec = list(map(int, rec))
        
        obs = np.array(rec[1:-1])
        feature_columns = [0, 1, 2, 3, 4, 5, 6, 7]
        assert len(feature_columns) == self.N_OBSERVATIONS
        obs = obs[feature_columns ]
        

        return obs

    def step(self, action):
        """
        Standard function for gym environment
        """
        action = CSPPEnvSocket.EXPAND_CHOICES[int(action)] # IMPORTANT
        str_b = f'step {action}'.encode('ascii')
        self.s.sendall(str_b)
        rec = self.s.recv(self.BUFFER_SIZE)
        rec = rec.decode('ascii')

        rec = rec.strip('step ok ')
        rec = rec.split(' ')
        rec = list(map(int, rec))
        self.step_info = rec # for callback recording

        done = True if rec[0] else False

        obs = np.array(rec[1:-1])
        obs = obs[[0, 1, 2, 3, 4, 5, 6, 7] ]
        
        # reward = rec[-1] + -1 * rec[1] * 0.2 
        reward =  -1 * rec[1] + (-10) # here add 10
        return obs, reward, done, {}

    
class CustomCallback(BaseCallback):
    def __init__(self, env, verbose=0):
        super(CustomCallback, self).__init__(verbose)
        self.env = env
        self.episode_info = []
        self.time_macros_deque = deque(maxlen=1)
        self.label_num_deque = deque(maxlen=1)

    def _on_step(self) -> bool:
        """
        Note: For off-policy algorithms like SAC, DDPG, TD3 or DQN, the notion of rollout corresponds to the steps taken in the environment between two updates.
        """
        done = self.env.step_info[0]
        self.episode_info.append( self.env.step_info )

        if done:
            self.episode_info = np.array(self.episode_info)
            time_macros = self.episode_info[:, 1].sum() # positive
            label_num = self.episode_info[:, -1].sum() * -1 # positive

            self.time_macros_deque.append(time_macros)
            self.label_num_deque.append(label_num)
            label_num = np.mean(self.label_num_deque)
            time_macros = np.mean(self.time_macros_deque)
            self.logger.record('rollout/label_num', label_num)
            self.logger.record('rollout/time_macros', time_macros)
            self.episode_info = []

        return True

    def _on_rollout_end(self) -> bool:
        return True


class TrainAgent(object):
    def __init__(self):

        self.ckpt_dir = ROOT_DIR/'outputs'/'checkpoints'
        self.query_file = ROOT_DIR/'inputs'/'queries'/'rquery_ne_rl'
        self.tensorboard_log_dir = ROOT_DIR/'outputs'/'tensorboard'
        self.queries = read_query(self.query_file)
        self.train_queries = self.queries[:5] # store queries for training
        self.eval_queries = self.queries[5:]
        self.q_idx = 0
        self.multi_query_train = True


    def train(self):
        u = None # any node
        v = None # any node
        cost_limit = None # cost

        env = CSPPEnvSocket(u, v, cost_limit, self.train_queries, self.multi_query_train)
        env.connect_socket()

        ckpt_prefix = f'dqn_c++env_{u}_{v}_multi_{self.multi_query_train}'
        tb_log_name = f'DQN_{u}_{v}_env'

        model = DQN("MlpPolicy", env, verbose=1,
                gamma=1,
                tau=0.5,
                exploration_fraction=0.2,
                exploration_final_eps=0.2,
                learning_starts=100000,
                target_update_interval=20000,
                buffer_size=500000,
                batch_size=32,
                policy_kwargs=dict(normalize_images=False, activation_fn=th.nn.ReLU, net_arch=[64, 64]),
                train_freq=(100, "step"),
                tensorboard_log=self.tensorboard_log_dir,
                )

        try:
            checkpoint_callback = CheckpointCallback(save_freq=10000, save_path=ROOT_DIR/'outputs'/'checkpoints',
                                                name_prefix=ckpt_prefix)
            extra_value_callback = CustomCallback(env)
            model.learn(total_timesteps=2000000,
                        callback=[checkpoint_callback, extra_value_callback],
                        tb_log_name=tb_log_name)
        except KeyboardInterrupt:
            env.close_socket()
            print('KeyboardInterrupt exit')
        except Exception as e:
            env.close_socket()
            print(e)

        env.close_socket()

    def eval(self):
        ckpt_name = None # Set a checkpoint name here. (checkpoints are stored during training)
        ckpt = self.ckpt_dir/ckpt_name
        query_idx = 0
        fix_action = False
        fix_action_value = 0
        substitute_action = False

        
        u = None # a node
        v = None # a node
        cost_limit = None # a cost_limit

        u = int(u); v = int(v); cost_limit = int(cost_limit)
        print('query: ', u, v, cost_limit)
        print('ckpt: ', ckpt)
        print('substitute action: ', substitute_action)

        model = DQN.load(ckpt)
        env = CSPPEnvSocket(u, v, cost_limit)
        env.connect_socket()

        obs = env.reset()
        epi_rewards = []
        epi_actions = []
        epi_time = []
        ft_size = []
        expand_num = []
        step = 0
        while True:
            action, _states = model.predict(obs, deterministic=True)
            action = fix_action_value if fix_action else action
            action = 4 if substitute_action else action

            obs, reward, done, info = env.step(action)
            
            epi_rewards.append(reward)
            epi_actions.append(action)
            epi_time.append(obs[0])
            print(f'step: {step}, ft: {obs[3]}, expand_num: {env.EXPAND_CHOICES[int(action)]}, reward: {reward}')
            step = step + 1
            ft_size.append(obs[3])
            expand_num.append(env.EXPAND_CHOICES[int(action)])
            if done:
                break
        
        print('epi reward ', -1*np.sum(epi_rewards))
        print('epi time ', np.sum(epi_time))
        print(f'query file {str(self.query_file.name)}')
        print(f'query_idx {query_idx} u {u} v {v} c {cost_limit}')
        env.close_socket()


    def save_policy(self):

        ckpt_name = None # Set a checkpoint name here
        ckpt = self.ckpt_dir/ckpt_name
        
        model = DQN.load(ckpt)
        q_net = model.policy.q_net.q_net
        q_net = q_net.to('cpu')

        for name, param in q_net.named_parameters():
            print(name, param.shape)

        # save as ScriptModule
        in_features = q_net[0].in_features
        example = torch.arange(0, in_features, dtype=torch.float32)
        traced_script_module = torch.jit.trace(q_net, example)
        traced_script_module.save(f"{ckpt_name}_traced_q_net.pt")



def read_query(query_file):
    queries = np.loadtxt(query_file, delimiter=' ', dtype=int, ndmin=2)
    queries = queries[:, [0, 1, 4]]
    return queries

def linear_schedule(initial_value: float):
    def func(progress_remaining: float) -> float:
        return max(1e-5, progress_remaining * initial_value)
    return func



trainer = TrainAgent()
trainer.train() # for training
# trainer.eval() # for evaluation
# trainer.save_policy() # for saving a checkpoint to a traced_q_net.pt file that could be used by the C++ CSPP() class.