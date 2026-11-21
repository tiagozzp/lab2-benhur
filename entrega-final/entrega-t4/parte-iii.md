# Trabalho 4 — Parte III

## Estruturas de dados

### 1. Registro de inovações (`individuo.c`)

- **Problema** — Para saber se uma sinapse já foi dividida, o programa procura o par de neurônios em um vetor de tamanho fixo.
- **Por que é um problema** — A busca fica mais demorada conforme o vetor cresce. Quando ele enche, o mesmo par pode receber IDs diferentes em redes distintas.
- **Alternativa** — Usar uma tabela hash que associa o par de IDs da origem e do destino ao ID do neurônio criado.
- **Por que melhora** — A consulta fica mais rápida e a tabela pode crescer para manter o registro dos novos pares.

### 2. Busca de neurônios por ID (`individuo.c`)

- **Problema** — Ao comparar ou cruzar redes, o programa percorre os neurônios para achar a posição de cada ID.
- **Por que é um problema** — Essa busca se repete para cada sinapse analisada, aumentando o trabalho conforme crescem as redes.
- **Alternativa** — Manter em cada rede uma tabela hash que associa o ID de cada neurônio à sua posição.
- **Por que melhora** — O programa pode consultar a posição pelo ID em vez de percorrer os neurônios a cada busca.

### 3. Armazenamento de espécies e indivíduos (`geracao.c` e `especie.c`)

- **Problema** — A geração guarda suas espécies em um vetor de tamanho fixo, e cada espécie faz o mesmo com seus indivíduos.
- **Por que é um problema** — Os vetores reservam espaço mesmo que tenham poucos elementos e só aceitam inserções até o limite definido.
- **Alternativa** — Usar vetores dinâmicos que aumentam de capacidade quando ficam cheios.
- **Por que melhora** — O espaço reservado acompanha melhor a quantidade de elementos, e o programa continua podendo acessar cada elemento pela posição.

## API do TAD Grafo

- **Limitação atual** — A interface de `grafo.h` permite percorrer as arestas de um nó por vez. Para dividir uma sinapse, o treinamento sorteia até 50 pares de neurônios e pode deixar de encontrar uma conexão válida mesmo quando existe uma.
- **Alteração proposta** — Acrescentar à API do TAD Grafo um iterador que percorra todas as arestas e informe a origem, o destino e o valor de cada uma.
- **Benefício para o treinamento** — O treinamento pode percorrer as conexões existentes e sortear uma entre as que podem ser divididas. O grafo fornece as arestas, e o treinamento define quais delas são válidas para a mutação.
