# 🏛️ Corte da Verdade
## 🎮 Jogo Educativo de Lógica Proposicional

[cite_start]O Corte da Verdade é um jogo interativo e educativo no qual o jogador assume o papel de um **réu** em um tribunal onde ninguém pode mentir[cite: 4]. [cite_start]Para vencer, você deve montar **tautologias** [cite: 5] [cite_start]usando variáveis e conectivos lógicos — e convencer o juiz de que seu argumento é irrefutável[cite: 17].

## 📌 Índice
* Visão Geral
* Tema e Narrativa
* Objetivo do Jogador
* Componentes do Jogo
* Níveis de Dificuldade
* Estrutura de uma Rodada
* Sistema de Pontuação
* Encerramento
* Equipe

---

## 🧠 Visão Geral
[cite_start]O jogador deve construir fórmulas de lógica proposicional utilizando[cite: 5]:

* [cite_start]**Variáveis**: p, q, r [cite: 5, 24]
* [cite_start]**Conectores**: $\lor$, $\land$, $\sim$, $\to$, $\leftrightarrow$ [cite: 5, 25]
* [cite_start]Parênteses [cite: 26]

[cite_start]Cada rodada apresenta peças limitadas [cite: 19][cite_start], e o jogador deve usar a lógica para montar tautologias — fórmulas verdadeiras em todas as interpretações possíveis[cite: 5, 17].

## 🎭 Tema e Narrativa
[cite_start]O jogo se passa no tribunal fictício chamado Corte da Verdade[cite: 7].

[cite_start]**Variáveis narrativas (Exemplo de Afirmações)**[cite: 8]:
* [cite_start]p: “O réu é culpado.” [cite: 9]
* [cite_start]q: “A testemunha está dizendo a verdade.” [cite: 11]
* [cite_start]r: “A prova apresentada é válida.” [cite: 12]

**Exemplos de fórmulas:**
* [cite_start]$p \lor \sim p$ — O réu é culpado ou não é culpado[cite: 14].
* [cite_start]$p \to q$ — Se o réu é culpado, então a testemunha está dizendo a verdade[cite: 16].

[cite_start]O juiz só aceita argumentos que sejam tautologias[cite: 17].

**Cada nível tem uma fórmula-alvo específica**.

## 🎯 Objetivo do Jogador
Você controla o **Réu**.
Em cada nível:

* [cite_start]Recebe um conjunto limitado de peças lógicas (variáveis e conectores)[cite: 19].
* [cite_start]Deve montar uma fórmula bem formada (FBF)[cite: 20].
* Tem **3 tentativas por nível** para acertar a tautologia-alvo.
* **Não é permitido repetir a mesma expressão no mesmo nível**.
* [cite_start]Mesmo que erre, sua última fórmula ainda gera pontos pela tabela verdade[cite: 22, 52].

**O jogo contém:**
* [cite_start]**3 níveis fáceis** (Nível 1 - Básico) [cite: 30]
* [cite_start]**3 níveis médios** (Nível 2 - Intermediário) [cite: 34]
* [cite_start]**3 níveis difíceis** (Nível 3 - Avançado) [cite: 37]
* Total: **9 desafios principais**

## 🧩 Componentes do Jogo
### [cite_start]Variáveis Proposicionais [cite: 24]
* p
* q
* r
### [cite_start]Conectores Lógicos [cite: 25]
* $\lor$ — ou
* $\land$ — e
* $\sim$ — não
* $\to$ — se... então...
* $\leftrightarrow$ — se e somente se
### Outros Elementos
* [cite_start]Parênteses [cite: 26]
* [cite_start]Sistema de pontuação [cite: 27]
* [cite_start]Dicas em linguagem natural [cite: 46]
* [cite_start]Dicas técnicas [cite: 50]

## 📊 Níveis de Dificuldade
| Nível | Variáveis | Linhas da tabela verdade | Exemplo |
| :---: | :-------: | :----------------------: | :------ |
| Básico | até 1 (p) | [cite_start]2 [cite: 63] | [cite_start]$p \lor \sim p$ [cite: 32] |
| Intermediário | até 2 (p, q) | [cite_start]4 [cite: 64] | [cite_start]$(p \land q) \to p$ [cite: 36] |
| Avançado | até 3 (p, q, r) | [cite_start]8 [cite: 65] | [cite_start]$(p \land (q \lor r)) \to p$ [cite: 39] |

## 🔁 Estrutura de uma Rodada
[cite_start]O jogo apresenta as peças disponíveis (variáveis, conectores e parênteses)[cite: 41].

[cite_start]Exemplo: p, p, $\lor$, $\sim$ $\to$ permite montar $p \lor \sim p$[cite: 42].

1.  [cite_start]O jogador faz a 1ª tentativa[cite: 43]:
    * [cite_start]Acertou $\to$ pontuação máxima[cite: 44, 55].
    * [cite_start]Errou $\to$ recebe dica em linguagem natural[cite: 46].
2.  [cite_start]2ª tentativa[cite: 48]:
    * [cite_start]Acertou $\to$ pontuação intermediária[cite: 49, 56].
3.  Se ainda errar:
    * [cite_start]Recebe dica mais técnica sobre tautologias[cite: 50].
4.  [cite_start]3ª tentativa[cite: 51]:
    * [cite_start]Acertou $\to$ menor pontuação[cite: 57].
5.  [cite_start]Caso não acerte em nenhuma das três[cite: 52]:
    * [cite_start]O jogo monta a tabela verdade da última fórmula construída[cite: 53, 60].
    * Cada linha **VERDADEIRA** (V) da tabela verdade vale **1 ponto extra**.

## 🏆 Sistema de Pontuação
### ✔️ Pontos por acerto
| Tentativa | Pontos |
| :---: | :---: |
| 1ª tentativa | **50 pts** |
| 2ª tentativa | **30 pts** |
| 3ª tentativa | **20 pts** |

### ❌ Se não acertar a fórmula-alvo
* Cada linha V da tabela verdade da última expressão = **1 ponto extra**.

### ⭐ Fase Bônus
* **Sofisma/Silogismo correto** $\to$ sua pontuação é **dobrada**.
* **Resposta errada** $\to$ você perde **metade dos pontos**.

## 🔚 Encerramento
Ao fim da sessão:

* [cite_start]Sua pontuação total é calculada[cite: 67].
* Você pode entrar no **Ranking (Top 10)**.

**Atalhos:**
* **ENTER / ESC** $\to$ voltar ao menu
* **ESC** durante o julgamento $\to$ retorna ao menu **mantendo sua pontuação**

[cite_start]Você pode jogar novamente para tentar superar seu próprio recorde de pontuação[cite: 69].

---

## 🧑‍🤝‍🧑 Equipe

A equipe responsável por este projeto é composta por:

- **Heitor Didier** — [Eito2511](https://github.com/Eito2511)  
- **Luiz Felipe** — [LuizMXavier](https://github.com/LuizMXavier)  
- **Marcus Vinicius** — [Marcus-Vini-Tavares](https://github.com/Marcus-Vini-Tavares)  
- **Nicolly Rodrigues** — [nicky89ck](https://github.com/nicky89ck)  
- **Pedro Armando** — [pedrosol-dev](https://github.com/pedrosol-dev)  
- **Thomaz Barros** — [JustaTBC](https://github.com/JustaTBC)  
