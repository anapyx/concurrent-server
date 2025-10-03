#include "logger.h"
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <iomanip>

// Essa função simula uma thread fazendo operações
void workerThread(int threadId, int numLogs) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay(10, 100);
    std::uniform_int_distribution<> levelDist(0, 3);
    
    Logger& logger = Logger::getInstance();
    
    for (int i = 0; i < numLogs; ++i) {
        // Simula um trabalho
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
        
        // Log com diferentes níveis
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

// Função do menu
void showMenu() {
    std::cout << "\n=== TESTE CLI - LIBTSLOG ===" << std::endl;
    std::cout << "1. Teste com múltiplas threads" << std::endl;
    std::cout << "2. Configurar arquivo de log" << std::endl;
    std::cout << "3. Configurar nível de log" << std::endl;
    std::cout << "4. Teste individual de níveis" << std::endl;
    std::cout << "5. Sair" << std::endl;
    std::cout << "Escolha uma opção: ";
}

// Teste com multiplas threads fazendo logs simultaneamente
void testMultipleThreads() {
    int numThreads, logsPerThread;
    
    std::cout << "Número de threads (1-20): ";
    std::cin >> numThreads;
    if (numThreads < 1 || numThreads > 20) {
        std::cout << "Número inválido. Usando 5 threads." << std::endl;
        numThreads = 5;
    }
    
    std::cout << "Logs por thread (1-50): ";
    std::cin >> logsPerThread;
    if (logsPerThread < 1 || logsPerThread > 50) {
        std::cout << "Número inválido. Usando 10 logs por thread." << std::endl;
        logsPerThread = 10;
    }
    
    Logger& logger = Logger::getInstance();
    logger.info("=== INICIANDO TESTE COM " + std::to_string(numThreads) + 
                " THREADS (" + std::to_string(logsPerThread) + " logs cada) ===");
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerThread, i + 1, logsPerThread);
    }
    
    // Aguarda todas as threads terminarem
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    logger.info("=== TESTE CONCLUÍDO EM " + std::to_string(duration.count()) + "ms ===");
    
    std::cout << "\nTeste concluído! Verificar logs acima e arquivo de log (se configurado)." << std::endl;
}

void configureLogFile() {
    std::string filename;
    std::cout << "Nome do arquivo de log (ou 'disable' para desabilitar): ";
    std::cin >> filename;
    
    if (filename != "disable") {
        Logger::getInstance().setLogFile(filename);
        Logger::getInstance().info("Arquivo de log configurado para: " + filename);
        std::cout << "Arquivo de log configurado para: " << filename << std::endl;
    } else {
        std::cout << "Saída para arquivo desabilitada." << std::endl;
    }
}

void configureLogLevel() {
    std::cout << "\nNíveis disponíveis:" << std::endl;
    std::cout << "0 - DEBUG" << std::endl;
    std::cout << "1 - INFO" << std::endl;
    std::cout << "2 - WARNING" << std::endl;
    std::cout << "3 - ERROR" << std::endl;
    std::cout << "Escolha o nível mínimo (0-3): ";
    
    int level;
    std::cin >> level;
    
    if (level >= 0 && level <= 3) {
        Logger::getInstance().setLevel(static_cast<Logger::Level>(level));
        Logger::getInstance().info("Nível de log configurado");
        std::cout << "Nível de log configurado com sucesso!" << std::endl;
    } else {
        std::cout << "Nível inválido!" << std::endl;
    }
}

void testIndividualLevels() {
    Logger& logger = Logger::getInstance();
    
    std::cout << "\nTestando todos os níveis de log..." << std::endl;
    
    logger.debug("Esta é uma mensagem de DEBUG - detalhes de execução");
    logger.info("Esta é uma mensagem de INFO - informação geral");
    logger.warning("Esta é uma mensagem de WARNING - atenção necessária");
    logger.error("Esta é uma mensagem de ERROR - problema detectado");
    
    std::cout << "Teste individual concluído!" << std::endl;
}

int main() {
    std::cout << "=== BIBLIOTECA LIBTSLOG - TESTE CLI ===" << std::endl;
    std::cout << "Sistema de logging thread-safe para aplicações concorrentes" << std::endl;
    
    // Configuração inicial
    Logger& logger = Logger::getInstance();
    logger.info("Sistema de teste inicializado");
    
    int option;
    do {
        showMenu();
        std::cin >> option;
        
        switch (option) {
            case 1:
                testMultipleThreads();
                break;
            case 2:
                configureLogFile();
                break;
            case 3:
                configureLogLevel();
                break;
            case 4:
                testIndividualLevels();
                break;
            case 5:
                logger.info("Sistema de teste finalizado");
                std::cout << "Saindo..." << std::endl;
                break;
            default:
                std::cout << "Opção inválida!" << std::endl;
                break;
        }
        
        if (option != 5) {
            std::cout << "\nPressione Enter para continuar...";
            std::cin.ignore();
            std::cin.get();
        }
        
    } while (option != 5);
    
    return 0;
}
