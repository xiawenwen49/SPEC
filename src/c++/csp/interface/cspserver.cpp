#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>

#include "csp/include/label.hpp"
#include "csp/include/cspp.hpp"
#include "csp/include/cspp_cpu.hpp"
#include "csp/include/args_parser.hpp"


#define BUFFERSIZE 512
void error(const char * msg)
{
    printf("%s\n", msg);
    exit(1);
}

std::vector<int> parse_buffer(char * buffer)
{
    std::vector<int> result;
    char * pch = strtok (buffer, " ");
    // printf("pch:%s\n", pch);
    while (pch != NULL) 
    {
        result.push_back( atoi(pch) );
        pch = strtok(NULL, " ");
    }

    return result;
}

string vec2string(std::vector<int> data)
{
    std::stringstream ss;
    for(int i = 0; i < data.size()-1; i++)
    {
        ss << data[i] << " ";
    }
    ss << data.back();

    std::string s = ss.str();
    return s;
}

class CSPServer {
    int socket_fd, socket_newfd, portno, n;
    socklen_t clilen;
    char buffer[BUFFERSIZE];
    struct sockaddr_in serv_addr, cli_addr;

    // csp
    CSPP<c_label, c_edge> *solver;

    public:

    CSPServer() {}

    void init_socket() 
    {   
        portno = 12345;

        socket_fd =  socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) 
            error("ERROR opening socket");
        
        // clear address structure
        bzero((char *) &serv_addr, sizeof(serv_addr));
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        
        if (bind(socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
        
        listen(socket_fd, 5);
        clilen = sizeof(cli_addr);
        printf("listening...\n");

    }


    void set_csp_solver(int argc, char *argv[])
    {
        cxxopts::ParseResult args = parse_args(argc, argv);
        int prune_type = args["prune"].as<int>();
        
        string config_file = args["config"].as<string>();
        string graph_file = args["graph_file"].as<string>();
        string query_file = args["query_file"].as<string>();
        int graph_type = args["graph_type"].as<int>();
        int ntask = args["ntask"].as<int>();
        int multi_thread = args["multithread"].as<int>();
        int withtruth = args["withtruth"].as<int>();
        int truth_position = args["truth_position"].as<int>();

        if (config_file != " ") {
            string graph_type_str;
            string ntask_str;
            
            ifstream conf_if (config_file.c_str());
            conf_if >> graph_type;
            conf_if >> graph_file;
            conf_if >> query_file;
            conf_if >> ntask_str;
            
            graph_type = stoi(graph_type_str);
            ntask = stoi(ntask_str);
            conf_if.close();

        }

        cout << "Graph file: " << graph_file << endl;
        cout << "Query file: " << query_file << endl;
        cout << "Prune type: " << prune_type << endl;
        cout << "Use multithread: " << multi_thread << endl;
        cout << "With csp truth: " << withtruth << endl;
        cout << "Compute truth label position: " << truth_position << endl;
        
        string graph_name = graph_file.substr( graph_file.find_last_of("/\\")+1 );

        this->solver = new CSPP<c_label, c_edge>(graph_file, graph_name, graph_type, query_file, ntask,
                prune_type, multi_thread, withtruth, truth_position);
        }



    void run() 
    {
        while (true) 
        {
            printf("waiting for connection...\n");
            socket_newfd = accept(socket_fd, (struct sockaddr *) &cli_addr, &clilen);
            if (socket_newfd < 0) 
                error("ERROR on accept");
            printf("connected to client port %d\n", ntohs(cli_addr.sin_port));

            while (true)
            {
                bzero(buffer, BUFFERSIZE);
                n = read(socket_newfd, buffer, BUFFERSIZE);
                if (n < 0) error("ERROR reading from socket");

                if ( strncmp(buffer, "close", 5) == 0 )
                {
                    close(socket_newfd);
                    printf("client socket closed\n");
                    break;
                }
                else if ( strncmp(buffer, "reset", 5) == 0)
                {
                    printf("reset: ");
                    std::vector<int> result = parse_buffer(buffer+5);
                    for (auto v : result)
                        printf("%d ", v);
                    printf("\n");
                    assert( result.size() == 3 );

                    std::vector<int> state = this->solver->reset( result[0], result[1], result[2] );
                    string state_str = vec2string(state);
                    state_str = string("reset ok ") + state_str;

                    bzero(buffer, BUFFERSIZE);
                    send(socket_newfd, state_str.c_str(), state_str.length(), 0);

                }
                else if ( strncmp(buffer, "step", 4) == 0)
                {
                    printf("step: ");
                    std::vector<int> result = parse_buffer(buffer+4);
                    for (auto v : result)
                        printf("%d ", v);
                    printf("\n");
                    assert( result.size() == 1 );

                    std::vector<int> state = this->solver->one_iteration( result[0] );
                    string state_str = vec2string(state);
                    state_str = string("step ok ") + state_str;

                    bzero(buffer, BUFFERSIZE);
                    send(socket_newfd, state_str.c_str(), state_str.length(), 0);
                }

            }

        }


    }

    void clean()
    {
        close(socket_fd);
        printf("\nmain socket closed\n");
    }

};

CSPServer csp_server;

void exit_handler(int s)
{
    csp_server.clean();
    exit(1); 
}

int main(int argc, char *argv[]) {
    signal(SIGINT, exit_handler);
    csp_server.init_socket();
    csp_server.set_csp_solver(argc, argv);
    csp_server.run();
    return 0;
}
