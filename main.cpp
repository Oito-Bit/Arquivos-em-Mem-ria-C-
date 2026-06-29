#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include "arquivos.h"

using namespace std;

int main()
{
    //IDENTIFICACAO atua como um gerador de Inodes (IDs únicos) para os arquivos.
    int IDENTIFICACAO= 0;
    //IDGLOBALDEUSUARIOS garante que cada usuário tenha um UID (User ID) único.
    int IDGLOBALDEUSUARIOS= 0;

    //Inicialização do grupo padrão. Todos os usuários pertencerão a este grupo para simplificar as checagens.
    GRUPO grupopadrao{};

    ///CRIAÇÃO DOS USUÁRIOS
    //O sistema define structs de usuários e atribui permissões simuladas via string (RWX).
    USUARIO root;
    root.id= IDGLOBALDEUSUARIOS++;
    root.nome= "root";
    root.permissao= "rwx"; //Acesso total
    root.grupo = &grupopadrao;

    USUARIO Elliot;
    Elliot.id= IDGLOBALDEUSUARIOS++;
    Elliot.nome= "elliot";
    Elliot.permissao= "rw-"; //Leitura e escrita, sem execução
    Elliot.grupo= &grupopadrao;

    USUARIO robot;
    robot.id= IDGLOBALDEUSUARIOS++;
    robot.nome= "robot";
    robot.permissao= "r--"; //Apenas leitura
    robot.grupo= &grupopadrao;

    ///ESTADO INICIAL DO FLUXO DO SISTEMA
    //USUARIOATUAL é uma cópia do usuário logado no momento. dita o fluxo das checagens de bitmask
    USUARIO USUARIOATUAL= robot;

    //RAIZ é o nó principal de diretórios
    DIRETORIO RAIZ;
    RAIZ.nome= "/";
    //DIRETORIOATUAL é o ponteiro de navegação
    DIRETORIO* DIRETORIOATUAL= &RAIZ;

    //String que armazenará o input bruto do usuário no console
    string COMANDODOUSUARIO;


    cout<< "SIMULADOR DE SISTEMA DE ARQUIVOS TIPO O DO LINUX\n"<< endl;
    cout<< "Logado como: "<< USUARIOATUAL.nome<< " | Diretorio Atual: "<< DIRETORIOATUAL->nome<<"\n\n";
    cout<< "mkdir nomedapasta - cria a pasta \n";
    cout<< "cd nome da pasta - vai para a pasta\n";
    cout<< "touch nome do arquivo  - cria o arquivo dentro da pasta\n";
    cout<< "echo escrito > nome do arquivo - escreve no arquivo\n";
    cout<< "cat nome do arquivo - le o arquivo\n";
    cout<< "rm nome do arquivo - exclui arquivo\n";
    cout<< "su root - tem permissoes rwx\n";
    cout<< "su elliot - tem permissoes rw-\n";
    cout<< "su robot - tem permisscoes r--\n";
    cout<<"cp <origem> <destino>\n";
    cout<<"mv <origem> <destino>\n";
    cout<< "exit - encerra\n\n";

    ///LOOP PRINCIPAL DO TERMINAL
    //Mantém o simulador rodando até receber um 'break'.
    while (true)
    {
        cout<< "\n"<< USUARIOATUAL.nome<< "@simulador:"<< DIRETORIOATUAL->nome<< "$ ";
        getline(cin, COMANDODOUSUARIO);

        if (COMANDODOUSUARIO.empty()) continue;

        ///COMANDO: exit -> Quebra o loop e encerra o programa.
        if (COMANDODOUSUARIO[0]== 'e'&& COMANDODOUSUARIO[1]== 'x'&& COMANDODOUSUARIO[2]== 'i'&& COMANDODOUSUARIO[3]== 't')
        {
            cout<< "Saindo do simulador..."<< endl;
            break;
        }
        ///COMANDO: su -> Troca de Usuário
        else if (COMANDODOUSUARIO.length() >= 2&& COMANDODOUSUARIO[0]== 's'&& COMANDODOUSUARIO[1]== 'u')
        {
            if (COMANDODOUSUARIO.length() <= 3)
            {
                cout<< "Erro: Informe o usuario. Ex: su root ou su robot"<< endl;
                continue;
            }
            //Extrai o nome do usuário desejado após os 3 primeiros caracteres
            string NOMEDEALVO= COMANDODOUSUARIO.substr(3);

            //Atualiza a variável de estado USUARIOATUAL, mudando o contexto de permissões de todo o sistema
            if (NOMEDEALVO== "root")
            {
                USUARIOATUAL= root;
                cout<< "Usuario alterado com sucesso! Logado como: root"<< endl;
            }
            else if (NOMEDEALVO== "elliot")
            {
                USUARIOATUAL= Elliot;
                cout<< "Usuario alterado com sucesso! Logado como: elliot"<< endl;
            }
            else if (NOMEDEALVO== "robot")
            {
                USUARIOATUAL= robot;
                cout<< "Usuario alterado com sucesso! Logado como: robot"<< endl;
            }
            else
            {
                cout<< "Erro: Usuario '"<< NOMEDEALVO<< "' nao cadastrado no sistema simulado."<< endl;
            }
        }

        ///COMANDO: mkdir -> Fluxo de Criação de Nós de Diretórios
        else if (COMANDODOUSUARIO[0]== 'm'&& COMANDODOUSUARIO[1]== 'k'&& COMANDODOUSUARIO[2]== 'd'&& COMANDODOUSUARIO[3]== 'i'&& COMANDODOUSUARIO[4]== 'r')
        {
            string NOMEDOPASTA= COMANDODOUSUARIO.substr(6); //Pega tudo depois de "mkdir "

            //Aloca dinamicamente um novo nó de diretório na memória
            DIRETORIO* NOVAPASTA= new DIRETORIO();
            NOVAPASTA->nome= NOMEDOPASTA;
            //Configura o ponteiro pai apontando para a pasta atual (encadeamento bidirecional)
            NOVAPASTA->pai= DIRETORIOATUAL;

            //Insere o novo nó na lista de filhos do diretório atual
            DIRETORIOATUAL->subdiretorios.push_back(NOVAPASTA);
            cout<< "Diretorio '"<< NOMEDOPASTA<< "' criado com sucesso."<< endl;
        }

        ///COMANDO: cd -> Fluxo de Navegação
        else if (COMANDODOUSUARIO[0]== 'c'&& COMANDODOUSUARIO[1]== 'd')
        {
            if (COMANDODOUSUARIO.length() <= 2)
            {
                cout << "Erro: Argumento ausente.\n";
                continue;
            }
            string ALVO= COMANDODOUSUARIO.substr(3); //Pega o nome da pasta

            if (ALVO== "..")
            {
                //Navega para cima na árvore usando o ponteiro 'pai'.
                //Protege contra NullPointer caso já esteja na RAIZ (onde o pai não existe).
                if (DIRETORIOATUAL->pai != nullptr)
                {
                    DIRETORIOATUAL= DIRETORIOATUAL->pai;
                }
                else
                {
                    cout<< "Erro: Voce ja esta no diretorio raiz '/'."<< endl;
                }
            }
            else
            {
                bool ACHOU= false;
                //Itera pelo vetor de subdiretórios tentando casar os nomes.
                for (DIRETORIO* SUB : DIRETORIOATUAL->subdiretorios)
                {
                    if (SUB->nome== ALVO)
                    {
                        //Ao achar, atualiza o ponteiro de estado de navegação.
                        DIRETORIOATUAL= SUB;
                        ACHOU= true;
                        break;
                    }
                }
                if (!ACHOU) cout<< "Erro: Diretorio '"<< ALVO<< "' nao encontrado."<< endl;
            }
        }

        ///COMANDO: touch -> Fluxo de Instanciação de um FCB (File Control Block) / Inode vazio
        else if (COMANDODOUSUARIO[0]== 't'&& COMANDODOUSUARIO[1]== 'o'&& COMANDODOUSUARIO[2]== 'u'&& COMANDODOUSUARIO[3]== 'c'&& COMANDODOUSUARIO[4]== 'h')
        {
            if (COMANDODOUSUARIO.length() <= 5)
            {
                cout << "Erro: Argumento ausente.\n";
                continue;
            }
            string NOMEARQUIVO= COMANDODOUSUARIO.substr(6);

            //Cria uma struct de arquivo, injeta os metadados (dono, grupo) e herda a máscara de permissões
            //convertida em inteiro bit a bit a partir da string de perfil do usuário logado.
            ARQUIVO NOVOARQ;
            NOVOARQ.nome= NOMEARQUIVO;
            NOVOARQ.conteudo= ""; //Nasce vazio
            NOVOARQ.tamanho= 0;
            NOVOARQ.tipo= texto; //Default
            NOVOARQ.id= IDENTIFICACAO++;
            NOVOARQ.iddodono= USUARIOATUAL.id;
            NOVOARQ.iddogrupo= USUARIOATUAL.grupo->id;
            NOVOARQ.permissaobitmask= CONVERTERSTRINGPARABITMASK(USUARIOATUAL.permissao);

            //Salva a estrutura na lista de arquivos do nó do diretório atual.
            DIRETORIOATUAL->listaarquivos.push_back(NOVOARQ);
            cout<< "Arquivo '"<< NOMEARQUIVO<< "' criado (touch) com ID/Inode: "<< NOVOARQ.id<< endl;
        }

        ///COMANDO: echo -> Fluxo de Escrita no Disco
        else if (COMANDODOUSUARIO[0]== 'e'&& COMANDODOUSUARIO[1]== 'c'&& COMANDODOUSUARIO[2]== 'h'&& COMANDODOUSUARIO[3]== 'o')
        {
            size_t SINALMAIOR= COMANDODOUSUARIO.find('>');
            //CORREÇÃO AQUI: Verifica se o '>' existe E se há caracteres suficientes depois dele (te achei bug fodido)
            if (SINALMAIOR == string::npos || SINALMAIOR + 2 >= COMANDODOUSUARIO.length())
            {
                cout<< "Erro: Sintaxe incorreta. Faltou o nome do arquivo. Use: echo <texto> > <arquivo>"<< endl;
                continue;
            }

            //Separa os dados em duas partes: carga e alvo
            string CONTEUDOINSERIDO= COMANDODOUSUARIO.substr(5, SINALMAIOR - 6);
            string NOMEALVO= COMANDODOUSUARIO.substr(SINALMAIOR + 2);

            bool ACHOU= false;
            for (ARQUIVO& ARQ : DIRETORIOATUAL->listaarquivos)
            {
                if (ARQ.nome== NOMEALVO)
                {
                    ACHOU= true;

                    ///CHECAGEM DE SEGURANÇA: Operação 2 = ESCRITA
                    if (!VALIDARPERMISSAO(ARQ, USUARIOATUAL, 2))
                    {
                        cout<< "Erro: Permissao Negada. O usuario '"<< USUARIOATUAL.nome<< "' nao tem direito de escrita."<< endl;
                        break;
                    }

                    ///FLUXO DE ROLLBACK
                    //Cria um backup seguro do estado do arquivo antes de tentar alterá-lo.
                    string CONTEUDO_ANTIGO= ARQ.conteudo;
                    int TAMANHO_ANTIGO= ARQ.tamanho;
                    TIPODOARQUIVO TIPO_ANTIGO= ARQ.tipo;

                    //Aplica as novas propriedades temporariamente na memória (RAM simulada)
                    ARQ.conteudo= CONTEUDOINSERIDO;
                    ARQ.tamanho= CONTEUDOINSERIDO.length();

                    //Define metadados com base na composição da carga de dados enviada.
                    for (char T : ARQ.conteudo)
                    {
                        if (isalpha(T) || ispunct(T))
                        {
                            ARQ.tipo= texto;
                        }
                        else if (isdigit(T)&& T != '0'&& T != '1')
                        {
                            ARQ.tipo= dados;
                        }
                        else
                        {
                            ARQ.tipo= programa; //Mantém a lógica do enum
                        }
                    }

                    //Tenta gravar fisicamente no disco rígido
                    if (ALOCARCONTEUDONODISCO(ARQ))
                    {
                        cout<< "Conteudo gravado com sucesso. Tipo definido com base nos caracteres."<< endl;
                    }
                    else
                    {
                        ///ROLLBACK: Se o disco estiver cheio (retornou false),
                        //desfaz a alteração na RAM virtual para não corromper o arquivo lógico.
                        ARQ.conteudo= CONTEUDO_ANTIGO;
                        ARQ.tamanho= TAMANHO_ANTIGO;
                        ARQ.tipo= TIPO_ANTIGO;
                    }
                    break;
                }
            }
            if (!ACHOU) cout<< "Erro: Arquivo '"<< NOMEALVO<< "' nao encontrado."<< endl;
        }

        ///COMANDO: cat -> Leitura de Disco
        else if (COMANDODOUSUARIO[0]== 'c'&& COMANDODOUSUARIO[1]== 'a'&& COMANDODOUSUARIO[2]== 't')
        {
            string NOMEALVO= COMANDODOUSUARIO.substr(4);
            bool ACHOU= false;

            for (ARQUIVO& ARQ : DIRETORIOATUAL->listaarquivos)
            {
                if (ARQ.nome== NOMEALVO)
                {
                    ACHOU= true;
                    //CHECAGEM DE SEGURANÇA: Operação 1= LEITURA.
                    if (!VALIDARPERMISSAO(ARQ, USUARIOATUAL, 1))
                    {
                        cout<< "Erro: Permissao Negada. O usuario '"<< USUARIOATUAL.nome<< "' nao tem direito de leitura."<< endl;
                        break;
                    }

                    //Lê os metadados de alocação de blocos e exibe o conteúdo lógico.
                    cout<< "--- Exibindo conteudo do bloco "<< ARQ.bloco_inicial<< " ---"<< endl;
                    cout<< ARQ.conteudo<< endl;
                    break;
                }
            }
            if (!ACHOU) cout<< "Erro: Arquivo '"<< NOMEALVO<< "' nao encontrado."<< endl;
        }

        ///COMANDO: mv -> Fluxo de Movimentação
        else if(COMANDODOUSUARIO[0]== 'm'&& COMANDODOUSUARIO[1]== 'v')
        {
            //Localiza o espaço em branco para dividir os dois argumentos obrigatórios (Origem e Destino)
            size_t ESPACO = COMANDODOUSUARIO.find(' ',3);

            if(ESPACO == string::npos)
            {
                cout<<"Uso: mv <origem> <destino>\n";
                continue;
            }

            string ORIGEM = COMANDODOUSUARIO.substr(3, ESPACO-3);
            string DESTINO = COMANDODOUSUARIO.substr(ESPACO+1);

            //Delega o fluxo lógico para a função externa, enviando os ponteiros de contexto.
            MV(DIRETORIOATUAL,
               ORIGEM,
               DESTINO,
               USUARIOATUAL);
        }

        ///COMANDO: cp -> Duplicação Física
        else if(COMANDODOUSUARIO[0]== 'c'&& COMANDODOUSUARIO[1]== 'p')
        {
            //Separa a string do comando nos dois alvos
            size_t ESPACO = COMANDODOUSUARIO.find(' ',3);

            if(ESPACO == string::npos)
            {
                cout<<"Uso: cp <origem> <destino>\n";
                continue;
            }

            string ORIGEM = COMANDODOUSUARIO.substr(3, ESPACO-3);
            string DESTINO = COMANDODOUSUARIO.substr(ESPACO+1);

            //Delega o processamento da cópia para a função externa.
            //aloca um novo espaço no "hardware" e gera novo 'id'.
            CP(DIRETORIOATUAL,
               ORIGEM,
               DESTINO,
               USUARIOATUAL);
        }

        ///COMANDO: rm -> Desvinculação de Diretórios -> unlink()
        else if (COMANDODOUSUARIO[0]== 'r'&& COMANDODOUSUARIO[1]== 'm')
        {
            if (COMANDODOUSUARIO.length() <= 2)
            {
                cout << "Erro: Argumento ausente.\n";
                continue;
            }
            string NOMEALVO= COMANDODOUSUARIO.substr(3);
            bool ACHOU= false;

            for (size_t i= 0; i < DIRETORIOATUAL->listaarquivos.size(); ++i)
            {
                if (DIRETORIOATUAL->listaarquivos[i].nome== NOMEALVO)
                {
                    ACHOU= true;
                    //CHECAGEM DE SEGURANÇA: Operação 2 = ESCRITA.
                    if (!VALIDARPERMISSAO(DIRETORIOATUAL->listaarquivos[i], USUARIOATUAL, 2))
                    {
                        cout<< "Erro: Permissao Negada para excluir o arquivo."<< endl;
                        break;
                    }
                    //Remove o elemento estrutural da lista de arquivos vinculada ao nó atual.
                    DIRETORIOATUAL->listaarquivos.erase(DIRETORIOATUAL->listaarquivos.begin() + i);
                    cout<< "Arquivo '"<< NOMEALVO<< "' removido com sucesso."<< endl;
                    break;
                }
            }
            if (!ACHOU) cout<< "Erro: Arquivo/Diretorio nao encontrado."<< endl;
        }

        else
        {
            cout<< "Erro: Comando nao reconhecido."<< endl;
        }
    }

    return 0;
}
