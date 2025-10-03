# Servidor HTTP Concorrente

Um sistema de logging concorrente desenvolvido em C++ que demonstra conceitos de programação concorrente: threads, exclusão mútua e sincronização.

## Funcionalidades

- **Logging Thread-Safe**: Sistema que permite múltiplas threads gravarem logs simultaneamente sem corrupção
- **Múltiplos Níveis**: DEBUG, INFO, WARNING, ERROR com filtragem configurável
- **Timestamps Precisos**: Cada log inclui data/hora com milissegundos
- **Saída Dupla**: Console e arquivo simultaneamente
- **API Simples**: Interface clara e fácil de usar

## Requisitos

- Linux (Ubuntu/Debian recomendado)
- GCC com suporte a C++17
- Make ou CMake

```bash
# Instalar dependências (Ubuntu/Debian)
sudo apt install build-essential cmake g++
```

## Compilação e Execução

```bash
# Clonar e entrar no diretório
cd concurrent-server

# Compilar
make

# Executar
./build/libtslog_test
```

### Alternativa com CMake

```bash
mkdir -p build && cd build
cmake ..
make
./libtslog_test
```

## Como Usar

O programa oferece um menu para testar o sistema.

### Exemplo de Teste

1. Execute o programa
2. Escolha opção **1** (Teste com múltiplas threads)
3. Configure **5 threads** e **10 logs por thread**
4. Observe a sincronização: 50 mensagens organizadas sem corrupção

## Conceitos Demonstrados

- **Exclusão Mútua**: `std::mutex` protege escrita simultânea
- **RAII**: `std::lock_guard` para unlock automático
- **Thread Safety**: Singleton seguro para múltiplas threads
- **Sincronização**: Coordenação entre threads produtoras

## Arquivos Principais

- `include/logger.h` - Interface da biblioteca
- `src/logger.cpp` - Implementação thread-safe
- `src/main.cpp` - Programa de teste CLI
