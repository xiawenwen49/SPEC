#ifndef _predict_
#define _predict_

#include <LightGBM/c_api.h>
#include <iostream>
#include <vector>
#include <string>
#include <torch/script.h> // One-stop header.
#include <torch/torch.h>

struct Net : torch::nn::Module {
 Net(int64_t In, int64_t Mid, int64_t Out) : 
    linear1(register_module("0", torch::nn::Linear(In, Mid))),
    linear2(register_module("2", torch::nn::Linear(Mid, Mid))),
    linear3(register_module("4", torch::nn::Linear(Mid, Out)))
  {
    // bias1 = register_parameter("b1", torch::randn(M));
    // bias2 = register_parameter("b2", torch::randn(M));
    // bias3 = register_parameter("b3", torch::randn(7));
  }
  torch::Tensor forward(torch::Tensor input) {
    auto output = torch::relu( linear1(input) );
    output = torch::relu( linear2(output) );
    output = linear3(output);
    return output;

  }
  torch::nn::Linear linear1;
  torch::nn::Linear linear2;
  torch::nn::Linear linear3;
//   torch::Tensor bias1;
//   torch::Tensor bias2;
//   torch::Tensor bias3;
    
    void load_from_scriptmodule(const char * script_module)
    {
        // load parameters from a ScriptModule file
        torch::jit::script::Module module;
        try {
            // Deserialize the ScriptModule from a file using torch::jit::load().
            module = torch::jit::load(script_module);
        }
        catch (const c10::Error& e) {
            std::cerr << "error loading the model\n";
            return;
        }

        // assign parameters
        for (auto p : this->named_parameters())
        {
            for (auto p_m : module.named_parameters())
            {
                if (p_m.name == p.key() )
                {
                    p.value().copy_(p_m.value);
                    std::cout << p.key() << " matched. " << p.value().sizes() << std::endl;
                    break;
                }
            }

        }

    }
};

class Predictor {
    public:
    BoosterHandle handle;
    FastConfigHandle fast_config_handle;

    int feature_size;
    int out_num_iterations;


    std::string model_file;
    void* in_p;
    double out_result;
    int64_t out_len;

    public:
    Predictor(int feature_size, const char * model_file) : feature_size(feature_size), model_file(model_file) {
        // load_model
        int temp = LGBM_BoosterCreateFromModelfile(this->model_file.c_str(), &out_num_iterations, &handle);    
        std::cout <<"lgbm predictor loaded" <<std::endl;

        // initialize
        LGBM_BoosterPredictForMatSingleRowFastInit(handle, C_API_PREDICT_NORMAL, 0, -1, C_API_DTYPE_FLOAT32, feature_size, "", &fast_config_handle);
        // num_iterations == num_trees, https://lightgbm.readthedocs.io/en/latest/Parameters-Tuning.html
    }


    double predict(std::vector<float> * data) {
        
        in_p = static_cast<void*>(data->data());
        // int res = LGBM_BoosterPredictForMatSingleRow(handle, in_p, C_API_DTYPE_FLOAT32, feature_size, 1, C_API_PREDICT_NORMAL, 1, 1, "", &out_len, out_result);
        // int res = LGBM_BoosterPredictForMatSingleRow(handle, in_p, C_API_DTYPE_FLOAT32, feature_size, 1, C_API_PREDICT_NORMAL, 1, 1, "", &out_len, out_result);
        int res = LGBM_BoosterPredictForMatSingleRowFast(fast_config_handle, in_p, &out_len, &out_result);
        // return 1;
        return out_result;

    }

};



#endif