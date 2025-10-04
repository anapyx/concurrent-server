# Servidor HTTP Concorrente

Implementação de servidor HTTP concorrente em C++ demonstrando programação multithreaded e thread safety.

## Funcionalidades

- **Servidor HTTP/1.0** com suporte a arquivos estáticos
- **Pool de Threads** para processamento concorrente de conexões
- **Sistema de Logging Thread-Safe** (libtslog) com múltiplos níveis
- **Fila Thread-Safe** para gerenciamento de conexões (padrão produtor/consumidor)
- **Smart Pointers e RAII** para gerenciamento automático de recursos
- **Sincronização robusta** usando std::mutex, std::condition_variable e std::atomic
- **Sockets TCP** multiplataforma (Linux/Windows)

**Tecnologias:** C++17, CMake, std::thread, pthread

## Como compilar e executar

```bash
# Compilar
cd build
cmake .. && make

# Executar servidor HTTP
./build/concurrent-server --port 8080 --threads 4

# Testar sistema de logging
./build/concurrent-server --test-logger
```

## Como testar

```bash
# Terminal 1: Iniciar servidor
./build/concurrent-server

# Terminal 2: Testar com cliente
./build/test-client 127.0.0.1 8080

# Teste de carga (10 clientes simultâneos)
./build/load-test 127.0.0.1 8080 10 5

# Testar logging com múltiplas threads
./build/concurrent-server --test-logger --test-threads 10
```
