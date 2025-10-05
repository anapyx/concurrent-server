# Servidor HTTP Concorrente

Implementação de servidor HTTP concorrente em C++ demonstrando programação multithreaded e thread safety.

## Funcionalidades

- **Servidor HTTP/1.1** com suporte a arquivos estáticos
- **Keep-Alive** para reutilização de conexões TCP (múltiplas requisições por conexão)
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
mkdir -p build && cd build
cmake .. && make

# Executar servidor HTTP
./build/concurrent-server --port 8080 --threads 4

# Testar sistema de logging
./build/concurrent-server --test-logger
```

## Como testar

```bash
# Terminal 1: Iniciar servidor
./build/concurrent-server --docroot www

# Terminal 2: Testar com cliente
./build/test-client 127.0.0.1 8080

# Teste de carga (10 clientes simultâneos, 5 requests cada)
./build/load-test 127.0.0.1 8080 10 5

# Testar logging com múltiplas threads
./build/concurrent-server --test-logger --test-threads 10

# Testar keep-alive com curl
curl -v -H "Connection: keep-alive" http://localhost:8080/test.txt
```
