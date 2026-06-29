    DOCUMENTAÇÃO TÉCNICA DO SIMULADOR DE SISTEMA DE ARQUIVOS LINUX
Este documento descreve detalhadamente o design, a arquitetura de dados e o 
mapeamento de conceitos teóricos aplicados no desenvolvimento do Simulador de 
Sistema de Arquivos baseado em Linux em C++. O sistema simula operações de 
gerenciamento de armazenamento, controle de acesso e manipulação de árvores de 
diretórios e arquivos.


    1. INSTRUÇÕES DE COMPILAÇÃO E EXECUÇÃO
O simulador foi desenvolvido utilizando o padrão C++ moderno, dependendo apenas 
da biblioteca padrão da linguagem (STL). Ele pode ser compilado em ambientes 
Linux, macOS ou Windows (via MinGW/GCC).

Passo 1: Certifique-se de que os arquivos 'main.cpp', 'arquivos.cpp' e 
'arquivos.h' estejam no mesmo diretório.

Passo 2: Abra o terminal/CMD no diretório dos arquivos e execute o comando de 
compilação:
    g++ main.cpp arquivos.cpp -o simulador

Passo 3: Para executar o simulador, digite o comando correspondente ao seu SO:
    * No Linux / macOS:
        ./simulador
    * No Windows (Prompt de Comando / PowerShell):
        simulador.exe

    2. ESCOLHAS DE DESIGN E ESTRUTURAS DE DADOS
O sistema baseia-se em três estruturas principais e em variáveis globais que 
delimitam o hardware virtual:

A. O Disco Virtual Simulado:
   No arquivo 'arquivos.cpp', o armazenamento físico é abstraído por um array de 
   caracteres global e um ponteiro de empilhamento:
       char DISCOSIMULADO[TAMANHODODISCO]; (onde TAMANHODODISCO = 1024 bytes)
       int PROXIMOBLOCOLIVRE = 0;

B. Struct ARQUIVO (Abstração do Inode / FCB):
   Representa a unidade fundamental de dados. Ela armazena metadados essenciais:
   - string nome: Identificador nominal do arquivo.
   - string conteudo: Cópia em buffer de memória cache/RAM virtual.
   - int tamanho: Comprimento do conteúdo em bytes.
   - TIPODOARQUIVO tipo: Enumeração que categoriza o arquivo (texto, dados, programa).
   - int id: Identificador numérico único associado ao recurso (análogo ao Inode).
   - int iddodono / iddogrupo: Vinculação de propriedade para segurança.
   - int permissaobitmask: Máscara octal para controle de acesso RWX.
   - int bloco_inicial: O índice físico de início da gravação no DISCOSIMULADO.

C. Struct DIRETORIO (Árvore de Diretórios N-ária):
   Estruturada para permitir o crescimento hierárquico dinâmico por meio de 
   vetores e ponteiros:
   - string nome: Nome da pasta.
   - DIRETORIO* pai: Ponteiro para o nó superior na árvore. Permite a operação 
     de retorno 'cd ..' sem a necessidade de buscas lineares.
   - vector<ARQUIVO> listaarquivos: Lista contendo os arquivos armazenados no nó.
   - vector<DIRETORIO> subdiretorios: Lista contendo os nós filhos (subpastas).

A escolha do 'vector' garante alocação dinâmica facilitada, enquanto o uso do 
ponteiro '*pai' estabelece uma árvore encadeada bidirecional extremamente eficiente.


    3. DISCUSSÃO DOS CONCEITOS TEÓRICOS DE SISTEMAS OPERACIONAIS
O projeto serve como um laboratório prático para demonstrar os principais pilares 
da teoria de Gerenciamento de Arquivos de um Sistema Operacional:

Conceito de Arquivo e Seus Atributos:
  Um arquivo é uma coleção lógica de informações correlatas gravadas em memória 
  secundária. O simulador demonstra isso ao isolar os dados lógicos (conteúdo) de 
  seus atributos reguladores (metadados). Elementos como tamanho, tipo de arquivo, 
  identificação de propriedade e timestamps conceituais são demonstrados na struct 
  ARQUIVO, evidenciando como o SO enxerga o arquivo além dos seus bytes internos.

Operações com Arquivos:
  As chamadas de sistema (System Calls) clássicas do SO são replicadas através de 
  funções modulares protegidas por validação de hardware virtual:
  - Criação (touch): Aloca uma nova struct com atributos padrão e ID único.
  - Escrita (echo >): Reescreve o conteúdo avaliando restrições de espaço e tipo 
    caractere. Possui mecanismo de ROLLBACK caso falte espaço físico no meio da 
    operação, mantendo o arquivo estável se a transação falhar.
  - Leitura (cat): Acessa os metadados do arquivo, localiza os índices e faz a 
    leitura em tela dos blocos.
  - Exclusão (rm): Apaga o registro lógico da lista de diretórios do nó atual.
  - Cópia e Movimentação (cp / mv): Demonstram a duplicação física com nova 
    alocação e a alteração nominal/registro de ponteiros, respectivamente.

O File Control Block (FCB) e o Inode Simulado:
  Em sistemas UNIX-like, o Inode guarda todas as informações estruturais do arquivo, 
  exceto seu nome. No simulador, o campo 'int id' gerado via ID incremental 
  funciona como o número do Inode. A struct ARQUIVO atua diretamente como o FCB, 
  agrupando as permissões e o vetor de blocos que o SO necessita para orquestrar 
  o ciclo de vida do arquivo.

Estrutura de Diretórios em Árvore e Suas Vantagens:
  O sistema adota uma árvore hierárquica a partir do nó raiz '/'. Esta abordagem 
  traz três vantagens teóricas nítidas observadas no uso do simulador:
  1. Eficiência: A busca por um arquivo limita-se ao escopo do nó atual ou do 
     caminho relativo percorrido, evitando varreduras lineares globais em discos grandes.
  2. Nomeação: Usuários diferentes ou o mesmo usuário podem criar arquivos com o 
     mesmo nome (ex: 'elliot.txt'), desde que estejam alocados em subdiretórios 
     distintos (ex: '/' e '/pasta_elliot'), solucionando colisões de nomes.
  3. Agrupamento: Permite organizar arquivos logicamente por projetos, tipos ou 
     níveis de segurança do sistema.

Mecanismo de Proteção de Acesso (RWX e Bitmasks):
  O sistema implementa o modelo tradicional de proteção UNIX baseado em três 
  esferas: Proprietário (Dono), Grupo e Outros. No código, isso é estruturado e 
  validado por meio de máscaras de bits (bitmasks) octais:
      LER_DONO = 0400 (em binário: 100000000)
      ESCREVER_DONO = 0200 (em binário: 010000000)
      EXECUTAR_DONO = 0100 (em binário: 001000000)
  A validação ocorre via operadores bit-a-bit (Bitwise AND '&'). Por exemplo, se 
  o tipo de operação requisitada for Escrita (código interno 2), o simulador avalia:
      if (MASCARANECESSARIA & ESCREVER_DONO)
  Se o resultado for diferente de zero, o acesso é concedido. Caso contrário, 
  retorna-se a mensagem de "Permissão Negada", blindando os dados lógicos do arquivo 
  contra modificações não autorizadas de usuários como o 'robot' (configurado como r--).

Simulação da Alocação de Blocos:
  A função 'ALOCARCONTEUDONODISCO' simula nativamente um esquema de Alocação Contígua 
  no array global estático. O arquivo recebe um endereço físico sequencial de início 
  ('bloco_inicial = PROXIMOBLOCOLIVRE') e ocupa posições contíguas adjacentes até o seu 
  tamanho final. A validação rígida de hardware impede a fragmentação destrutiva ou 
  o estouro físico, garantindo integridade e simulando o comportamento de partições 
  reais em sistemas embarcados e de armazenamento primário.

    4. EXEMPLOS DE USO E COMPARAÇÃO COM COMANDOS LINUX REAIS
A tabela abaixo correlaciona o comportamento observado no simulador com a resposta 
esperada em um terminal de uma distribuição GNU/Linux real (como Ubuntu ou Debian):

+-----------------------+--------------------------------+--------------------------------+
| Comando Executado     | Comportamento no Simulador     | Comportamento no Linux Real    |
+-----------------------+--------------------------------+--------------------------------+
| su elliot             | Altera a variável USUARIOATUAL  | Alterna a sessão de usuário    |
|                       | mudando as checagens de bitmask| via chamada setuid / PAM.      |
+-----------------------+--------------------------------+--------------------------------+
| touch arq.txt         | Cria o arquivo vazio no vector | Cria o arquivo alocando um     |
|                       | informando o ID/Inode criado.  | número de Inode vago na tabela.|
+-----------------------+--------------------------------+--------------------------------+
| echo teste > arq.txt  | Grava os caracteres de forma   | Grava os blocos de dados nos   |
|                       | contígua no DISCOSIMULADO.     | setores do bloco do HD/SSD.    |
+-----------------------+--------------------------------+--------------------------------+
| cat arq.txt           | Exibe a mensagem de leitura do | Lê os blocos apontados pelo    |
|                       | bloco e cospe os caracteres.   | Inode do arquivo para o stdout.|
+-----------------------+--------------------------------+--------------------------------+
| cp arq.txt copia.txt  | Verifica o tamanho livre no    | Realiza a leitura e escrita do |
|                       | disco; se houver, clona o nó.  | conteúdo gerando novo Inode.   |
+-----------------------+--------------------------------+--------------------------------+
| mv arq.txt b.txt      | Altera a string 'nome' dentro  | Altera a entrada de diretório  |
|                       | da struct ARQUIVO na lista.    | mantendo o mesmo número Inode. |
+-----------------------+--------------------------------+--------------------------------+
| rm b.txt              | Remove o elemento da lista de  | Desvincula o link do arquivo;  |
|                       | arquivos limpando a referência.| libera os blocos se linkcnt=0. |
+-----------------------+--------------------------------+--------------------------------+
| cd ..                 | Atualiza o ponteiro atual para | Altera o diretório de trabalho |
|                       | o ponteiro armazenado em 'pai' | do processo para a pasta pai.  |
+-----------------------+--------------------------------+--------------------------------+

Com isso, o simulador cumpre de forma integral os requisitos acadêmicos e práticos 
da análise comportamental de Sistemas de Arquivos e Gerenciamento de Recursos.
================================================================================
