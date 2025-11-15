# 🏛️ Corte da Verdade: Um Jogo de Lógica Proposicional

## 📌 Descrição do Projeto

**Corte da Verdade** é um jogo educativo e interativo baseado em **lógica proposicional**. O jogador assume o papel de um réu em um tribunal onde a regra fundamental é: **ninguém pode mentir**.

O objetivo central do jogo é construir fórmulas lógicas que sejam **tautologias** (argumentos sempre verdadeiros) para provar a irrefutabilidade da sua defesa perante o juiz.

O projeto visa combinar raciocínio lógico, narrativa temática e um sistema de pontuação para incentivar o aprendizado progressivo dos conceitos de lógica.

---

## 🧠 Conceitos Fundamentais

O jogador deve manipular os seguintes elementos para construir suas fórmulas:

### 1. Variáveis Proposicionais (Afirmações do Caso)
* **p:** “O réu é culpado.”
* **q:** “A testemunha está dizendo a verdade.”
* **r:** “A prova apresentada é válida.”

### 2. Conectores Lógicos
| Símbolo | Nome | Representação |
| :---: | :--- | :--- |
| **v** | OU | Disjunção |
| **^** | E | Conjunção |
| **~** | NÃO | Negação |
| **->** | Implicação | Condicional |
| **<->** | Bicondicional | Dupla Implicação |

**Regra de Validade:** A fórmula só é aceita se for uma **tautologia** (Verdadeira em todas as linhas da Tabela Verdade).

---

## 🧩 Como Funciona Cada Rodada

O jogador recebe um conjunto limitado de peças lógicas (variáveis e conectores) e tem **3 tentativas** para criar uma tautologia.

### Fluxo e Dicas
1.  **Montagem:** O jogador constrói uma fórmula bem formada (Ex: `p v ~p`).
2.  **1ª Tentativa:**
    * **Acerto:** Pontuação máxima. Rodada encerrada.
    * **Erro:** Recebe uma **dica em linguagem natural**.
3.  **2ª Tentativa:**
    * **Acerto:** Pontuação intermediária.
    * **Erro:** Recebe uma **dica mais técnica** (sobre leis lógicas, como De Morgan, etc.).
4.  **3ª Tentativa:** Último esforço para obter a tautologia.

---

## 📈 Sistema de Pontuação

### ✔️ Pontuação por Tautologias
| Tentativa de Acerto | Pontuação |
| :---: | :---: |
| 1ª tentativa | **50 pontos** |
| 2ª tentativa | **30 pontos** |
| 3ª tentativa | **20 pontos** |

### ✔️ Pontuação por Linhas Verdadeiras (Se falhar)
Se a fórmula final não for uma tautologia, o jogador ganha **1 ponto por cada linha verdadeira (V)** na Tabela Verdade da última fórmula submetida.

| Variáveis na Fórmula | Número de Linhas (Total) |
| :---: | :---: |
| 1 variável ($p$) | 2 linhas |
| 2 variáveis ($p, q$) | 4 linhas |
| 3 variáveis ($p, q, r$) | 8 linhas |

---

## 🔥 Níveis de Dificuldade

O nível de dificuldade é determinado pelo número de variáveis permitidas na construção da fórmula:

| Nível | Variáveis Permitidas | Exemplo de Tautologia |
| :---: | :---: | :--- |
| **1 – Básico** | 1 variável | `p v ~p` |
| **2 – Intermediário** | 2 variáveis | `(p ^ q) -> p` |
| **3 – Avançado** | 3 variáveis | `(p ^ (q v r)) -> p` |

---

## 🏁 Encerramento

Após um conjunto de rodadas, a pontuação total da sessão é somada, permitindo ao jogador acompanhar seu desempenho lógico e tentar superar seus próprios recordes.
---

## 🧑‍🤝‍🧑 Equipe

A equipe responsável por este projeto é composta por:

- **Heitor Didier** — [Eito2511](https://github.com/Eito2511)  
- **Luiz Felipe** — [LuizMXavier](https://github.com/LuizMXavier)  
- **Marcus Vinicius** — [Marcus-Vini-Tavares](https://github.com/Marcus-Vini-Tavares)  
- **Nicolly Rodrigues** — [nicky89ck](https://github.com/nicky89ck)  
- **Pedro Armando** — [pedrosol-dev](https://github.com/pedrosol-dev)  
- **Thomaz Barros** — [JustaTBC](https://github.com/JustaTBC)  
