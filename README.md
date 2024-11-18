# Projeto de Supercomputação de Bioinformática

### **Aluno:** Alberto Mansur

---

## **Descrição do Projeto**
Este projeto tem como objetivo explorar técnicas de programação paralela para análise de dados genéticos em bioinformática, utilizando **OpenMP** e **MPI**. Foram abordadas diversas tarefas relacionadas à manipulação de sequências genéticas de DNA e RNA, divididas em quatro exercícios:

1. **Contagem de Bases**  
   Contar o número de ocorrências de cada base (A, T, C, G) em uma grande sequência de DNA.

2. **Transcrição de DNA para RNA**  
   Converter uma sequência de DNA em RNA substituindo as bases 'T' por 'U'.

3. **Trabalhando com Aminoácidos**  
   Contar quantas proteínas foram inicializadas a partir de códons `AUG`.

4. **Tradução de RNA em Proteínas**  
   Traduzir uma sequência de RNA em aminoácidos correspondentes, até encontrar um códon STOP.

---

## **Tecnologias Utilizadas**
- **MPI (Message Passing Interface):**
  Utilizado para dividir os dados entre processos diferentes, possibilitando processamento em memória distribuída em clusters.
  
- **OpenMP (Open Multi-Processing):**
  Utilizado para paralelizar tarefas dentro de um mesmo nó, aproveitando múltiplos threads em máquinas com memória compartilhada.

---


# Comparação entre OpenMP e MPI nos Exercícios

## **Exercício 1: Contagem de Bases**
**Descrição:** Contar o número de ocorrências de cada base (A, T, C, G) em uma grande cadeia de DNA.

### **Uso do OpenMP**
- **Vantagens:**
  - Permite paralelizar facilmente o loop que percorre os caracteres da sequência.
  - É ideal para uso em máquinas com memória compartilhada (ex.: servidores multicore).
  - Menor overhead de comunicação, pois todos os threads compartilham a mesma memória.
- **Desvantagens:**
  - Limitado ao hardware com memória compartilhada (não escalável para clusters).

### **Uso do MPI**
- **Vantagens:**
  - Distribui a sequência entre múltiplos processos, permitindo processamento em memória distribuída (clusters).
  - Escalável para grandes volumes de dados e várias máquinas.
- **Desvantagens:**
  - A comunicação entre processos (ex.: transmissão de dados com `MPI_Bcast` e `MPI_Reduce`) adiciona overhead.
  - Mais complexo de implementar.

### **Conclusão:**
Para contagem de bases em um único servidor, **OpenMP** é mais eficiente e simples. No entanto, para processar grandes volumes de dados em um cluster, **MPI** é mais adequado, pois permite distribuir o processamento entre várias máquinas.

---

## **Exercício 2: Transcrição de DNA em RNA**
**Descrição:** Converter uma sequência de DNA em RNA substituindo 'T' por 'U'.

### **Uso do OpenMP**
- **Vantagens:**
  - Paraleliza facilmente o loop de substituição (ex.: substituir 'T' por 'U' em cada posição).
  - Evita overhead de comunicação, pois tudo ocorre em memória compartilhada.
- **Desvantagens:**
  - Limitado a máquinas com memória compartilhada.

### **Uso do MPI**
- **Vantagens:**
  - Permite dividir a sequência em partes entre processos, processando cada parte independentemente.
  - Escalável para grandes volumes de dados em sistemas distribuídos.
- **Desvantagens:**
  - Adiciona overhead para comunicação entre processos, especialmente ao reunir as partes convertidas.

### **Conclusão:**
**OpenMP** é ideal para este exercício quando os dados são pequenos ou médios e a execução ocorre em um único servidor. Para transcrição de genomas completos ou em clusters, **MPI** é mais eficiente.

---

## **Exercício 3: Trabalhando com Aminoácidos**
**Descrição:** Contar quantas proteínas foram inicializadas (códons `AUG`).

### **Uso do OpenMP**
- **Vantagens:**
  - Paraleliza eficientemente o loop de busca por códons de início (`AUG`).
  - Fácil de implementar, pois cada thread busca independentemente na sequência.
- **Desvantagens:**
  - Restrito a máquinas com memória compartilhada.

### **Uso do MPI**
- **Vantagens:**
  - Divide a sequência entre processos, cada um buscando códons na sua parte.
  - Escalável para sequências muito grandes (ex.: genomas completos).
- **Desvantagens:**
  - Overhead na comunicação ao combinar os resultados (uso de `MPI_Reduce`).

### **Conclusão:**
Para sequências menores, **OpenMP** é preferido pela simplicidade. Para processar genomas completos distribuídos em clusters, **MPI** é essencial devido à escalabilidade.

---

## **Exercício 4: Tradução para Proteínas**
**Descrição:** Traduzir uma sequência de RNA em aminoácidos até encontrar um códon STOP.

### **Uso do OpenMP**
- **Vantagens:**
  - Paraleliza a tradução de códons de maneira eficiente.
  - Cada thread pode processar segmentos diferentes do RNA.
  - Ideal para máquinas com memória compartilhada.
- **Desvantagens:**
  - Menos flexível para escalabilidade em clusters.

### **Uso do MPI**
- **Vantagens:**
  - Divide a sequência entre processos MPI, permitindo tradução em paralelo em sistemas distribuídos.
  - Permite lidar com sequências muito grandes, processando partes em diferentes máquinas.
- **Desvantagens:**
  - Mais complexo de implementar devido à necessidade de combinar os resultados com `MPI_Gatherv`.

### **Conclusão:**
**MPI** é mais indicado para este exercício, especialmente para grandes sequências de RNA em clusters. **OpenMP** é mais simples e eficiente para máquinas com memória compartilhada e sequências menores.

---

## **Considerações Gerais**

### **OpenMP**
- Melhor para paralelismo dentro de uma única máquina com memória compartilhada.
- Menor overhead de implementação e execução.
- Ideal para dados de tamanho moderado.

### **MPI**
- Melhor para sistemas distribuídos (clusters) com memória distribuída.
- Escalável para grandes volumes de dados.
- Ideal para genomas completos ou sequências extremamente grandes.

### **Híbrido (MPI + OpenMP)**
- Combina o melhor dos dois mundos:
  - **MPI** distribui os dados entre máquinas.
  - **OpenMP** paraleliza o processamento dentro de cada máquina.
- Ideal para sistemas heterogêneos e clusters multicore.
