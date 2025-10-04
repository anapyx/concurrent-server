#include "http_server.h"
#include "logger.h"
#include "cli.h"
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <csignal>
#include <memory>

std::unique_ptr<HttpServer> g_server;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        Logger::getInstance().info("Sinal de encerramento recebido");
        if (g_server) {
            g_server->stop();
        }
        exit(0);
    }
}

// Função para testar thread safety do logger
void workerThread(int threadId, int numLogs) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay(10, 100);
    std::uniform_int_distribution<> levelDist(0, 3);
    
    Logger& logger = Logger::getInstance();
    
    for (int i = 0; i < numLogs; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
        
        Logger::Level level = static_cast<Logger::Level>(levelDist(gen));
        std::string message = "Thread " + std::to_string(threadId) + " - Operação " + std::to_string(i + 1);
        
        switch (level) {
            case Logger::Level::DEBUG:
                logger.debug(message + " (processando dados...)");
                break;
            case Logger::Level::INFO:
                logger.info(message + " (operação concluída com sucesso)");
                break;
            case Logger::Level::WARNING:
                logger.warning(message + " (aviso: recurso limitado)");
                break;
            case Logger::Level::ERROR:
                logger.error(message + " (erro: falha na operação)");
                break;
        }
    }
    
    logger.info("Thread " + std::to_string(threadId) + " finalizou todas as operações");
}

void runLoggerTests(int numThreads, int logsPerThread) {
    Logger::getInstance().info("=== Iniciando Testes de Thread Safety do libtslog ===");
    Logger::getInstance().info("Threads: " + std::to_string(numThreads) + 
                              ", Logs por thread: " + std::to_string(logsPerThread));
    
    auto startTime = std::chrono::steady_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerThread, i + 1, logsPerThread);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    Logger::getInstance().info("=== Testes Concluídos ===");
    Logger::getInstance().info("Tempo total: " + std::to_string(duration.count()) + "ms");
    Logger::getInstance().info("Total de logs: " + std::to_string(numThreads * logsPerThread));
}

void printUsage() {
    std::cout << "Uso: ./concurrent-server [OPÇÕES]\n";
    std::cout << "Opções do Servidor HTTP:\n";
    std::cout << "  -p, --port <porta>       Porta do servidor (padrão: 8080)\n";
    std::cout << "  -t, --threads <num>      Número de threads trabalhadoras (padrão: 4)\n";
    std::cout << "  -d, --docroot <caminho>  Diretório raiz dos documentos (padrão: ./www)\n";
    std::cout << "  -h, --help               Mostrar esta mensagem de ajuda\n";
    std::cout << "\nOpções de Teste:\n";
    std::cout << "  --test-logger            Executar apenas testes do sistema de logging\n";
    std::cout << "  --test-threads <num>     Número de threads para teste (padrão: 5)\n";
    std::cout << "  --test-logs <num>        Logs por thread no teste (padrão: 10)\n";
    std::cout << "\nExemplos:\n";
    std::cout << "  ./concurrent-server                           # Iniciar servidor HTTP\n";
    std::cout << "  ./concurrent-server --port 9090 --threads 8  # Servidor personalizado\n";
    std::cout << "  ./concurrent-server --test-logger             # Testar apenas logging\n";
    std::cout << "  ./concurrent-server --test-logger --test-threads 10  # Teste com 10 threads\n";
}

int main(int argc, char* argv[]) {
    CLI cli(argc, argv);
    
    if (cli.hasFlag("--help") || cli.hasFlag("-h")) {
        printUsage();
        return 0;
    }
    
    // Teste do sistema de logging
    if (cli.hasFlag("--test-logger")) {
        int numThreads = cli.getIntOption("--test-threads", 5);
        int logsPerThread = cli.getIntOption("--test-logs", 10);
        
        runLoggerTests(numThreads, logsPerThread);
        return 0;
    }
    
    // Configuração do servidor HTTP
    int port = cli.getIntOption("--port", cli.getIntOption("-p", 8080));
    size_t numThreads = cli.getIntOption("--threads", cli.getIntOption("-t", 4));
    std::string documentRoot = cli.getStringOption("--docroot", 
                                cli.getStringOption("-d", "./www"));
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        Logger::getInstance().info("=== Servidor HTTP Concorrente ===");
        Logger::getInstance().info("Porta: " + std::to_string(port));
        Logger::getInstance().info("Threads: " + std::to_string(numThreads));
        Logger::getInstance().info("Diretório raiz: " + documentRoot);
        
        g_server = std::make_unique<HttpServer>(port, numThreads, 100, documentRoot);
        
        Logger::getInstance().info("Iniciando servidor...");
        if (!g_server->start()) {
            Logger::getInstance().error("Falha ao iniciar o servidor");
            return 1;
        }
        
    } catch (const std::exception& e) {
        Logger::getInstance().error("Erro do servidor: " + std::string(e.what()));
        return 1;
    }
    
    return 0;
}