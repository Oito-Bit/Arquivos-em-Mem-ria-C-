#include "arquivos.h"
#include <iostream>
#include <cctype>

using namespace std;

///SIMULAÇÃO DE HARDWARE
//armazenamento físico
char DISCOSIMULADO[TAMANHODODISCO];
//Ponteiro global que indica o próximo índice vazio
int PROXIMOBLOCOLIVRE= 0;

///CONVERSÃO DE PERMISSÕES
//Recebe uma string  e converte para com máscara de bits
int CONVERTERSTRINGPARABITMASK(string PERMISSAO)
{
    int MASCARA= 0;
    //Atribui os bits correspondentes ao Dono
    if (PERMISSAO[0]== 'r') MASCARA |= LER_DONO;
    if (PERMISSAO[1]== 'w') MASCARA |= ESCREVER_DONO;
    if (PERMISSAO[2]== 'x') MASCARA |= EXECUTAR_DONO;
    //Atribui as permissões correspondentes aos "Outros"
    if (PERMISSAO[0]== 'r') MASCARA |= LER_OUTROS;
    if (PERMISSAO[2]== 'x') MASCARA |= EXECUTAR_OUTROS;

    return MASCARA;
}

///CONTROLE DE ACESSO
//Compara a máscara com a operação
bool VALIDARPERMISSAO(ARQUIVO& ARQ, USUARIO& USUARIOATUAL, int TIPOOPERACAO)
{
    ///Usuário 'root' ignora todas as validações
    if (USUARIOATUAL.nome== "root") return true;

    int MASCARANECESSARIA= 0;

    //Verifica se o usuário que está tentando acessar é o dono
    if (USUARIOATUAL.id== ARQ.iddodono)
    {
        //Define qual bit precisamos testar
        if (TIPOOPERACAO== 1) MASCARANECESSARIA= LER_DONO;
        if (TIPOOPERACAO== 2) MASCARANECESSARIA= ESCREVER_DONO;
        if (TIPOOPERACAO== 3) MASCARANECESSARIA= EXECUTAR_DONO;
    }
    else
    {
        //Se não for o Dono, aplica as regras "Outros"
        if (TIPOOPERACAO== 1) MASCARANECESSARIA= LER_OUTROS;
        if (TIPOOPERACAO== 2) MASCARANECESSARIA= ESCREVER_OUTROS;
        if (TIPOOPERACAO== 3) MASCARANECESSARIA= EXECUTAR_OUTROS;
    }

    //Só retorna true se o bit exigido estiver ativado na máscara
    return (ARQ.permissaobitmask & MASCARANECESSARIA) != 0;
}

///GERENCIADOR DE ALOCAÇÃO DE DISCO PRA NÃO ENCHER
bool ALOCARCONTEUDONODISCO(ARQUIVO& ARQ)
{
    //Verifica se o tamanho do conteúdo ultrapassa o limite
    if (PROXIMOBLOCOLIVRE + ARQ.conteudo.length() > TAMANHODODISCO)
    {
        cout << "Erro: Espaco em disco insuficiente!\n";
        return false; // Falha na alocação, disco cheio
    }

    //Registra no FCB o índice inicial onde os dados começam
    ARQ.bloco_inicial= PROXIMOBLOCOLIVRE;

    //Realiza a gravação copiando char a char para o armazenamento
    for (size_t i= 0; i < ARQ.conteudo.length(); ++i)
    {
        DISCOSIMULADO[PROXIMOBLOCOLIVRE]= ARQ.conteudo[i];
        PROXIMOBLOCOLIVRE++; //Avança o ponteiro global de escrita
    }

    ///Tratamento estético "bloco 0 ate -1"
    if (ARQ.conteudo.length()== 0)
    {
        cout << "[DISCO] Arquivo '" << ARQ.nome << "' criado vazio (sem blocos alocados).\n";
    }
    else
    {
        cout << "[DISCO] Arquivo '" << ARQ.nome << "' alocado a partir do bloco "
             << ARQ.bloco_inicial << " ate " << PROXIMOBLOCOLIVRE - 1 << endl;
    }

    return true; // Sucesso na alocação física
}

///FUNÇÃO AUXILIAR DE BUSCA LÓGICA
//localizar um ponteiro do arquivo pelo nome
ARQUIVO* PROCURARARQUIVO(DIRETORIO* DIRETORIOATUAL, string NOME)
{
    for(unsigned int i= 0; i < DIRETORIOATUAL->listaarquivos.size(); i++)
    {
        //Se encontrar uma correspondência exata, retorna o endereço
        if(DIRETORIOATUAL->listaarquivos[i].nome== NOME)
        {
            return &DIRETORIOATUAL->listaarquivos[i];
        }
    }

    return nullptr; //Arquivo não encontrado
}

///Cópia
//Clona o arquivo lógica e fisicamente
void CP(DIRETORIO* DIRETORIOATUAL,
        string ORIGEM,
        string DESTINO,
        USUARIO& USUARIOATUAL)
{
    //Tenta encontrar o arquivo
    ARQUIVO* ARQORIGINAL=
        PROCURARARQUIVO(DIRETORIOATUAL, ORIGEM);

    if(ARQORIGINAL== nullptr)
    {
        cout<<"Erro: Arquivo nao encontrado.\n";
        return;
    }

    //CHECAGEM DE SEGURANÇA: Exige permissão 1 (Leitura)
    if(!VALIDARPERMISSAO(*ARQORIGINAL,
                         USUARIOATUAL,
                         1))
    {
        cout<<"Erro: Permissao negada para leitura.\n";
        return;
    }

    //Impede a cópia se já existir um arquivo com o mesmo nome
    if(PROCURARARQUIVO(DIRETORIOATUAL, DESTINO))
    {
        cout<<"Erro: Ja existe um arquivo com esse nome.\n";
        return;
    }

    //O NOVOARQUIVO nasce na memória instanciado com os mesmos metadados
    ARQUIVO NOVOARQUIVO;

    NOVOARQUIVO.nome= DESTINO;
    NOVOARQUIVO.conteudo= ARQORIGINAL->conteudo;
    NOVOARQUIVO.tamanho= ARQORIGINAL->tamanho;
    NOVOARQUIVO.tipo= ARQORIGINAL->tipo;
    NOVOARQUIVO.id= rand(); // Gera um novo Inode simulado
    NOVOARQUIVO.iddodono= USUARIOATUAL.id; // O novo dono é quem disparou o comando cp
    NOVOARQUIVO.iddogrupo= USUARIOATUAL.grupo->id;
    NOVOARQUIVO.permissaobitmask= ARQORIGINAL->permissaobitmask; // Preserva as restrições originais

    ///Proteção de disco: Solicita espaço em disco físico
    if(!ALOCARCONTEUDONODISCO(NOVOARQUIVO))
    {
        return; //Sai da função sem salvar o arquivo clonado no diretório
    }

    //Se passou, adiciona o novo vínculo lógico
    DIRETORIOATUAL->listaarquivos.push_back(
        NOVOARQUIVO
    );

    cout<<"Arquivo copiado com sucesso.\n";
}

///MV (MOVE / RENAME)
//Altera a propriedade lógica de nome sem duplicar
void MV(DIRETORIO* DIRETORIOATUAL,
        string ORIGEM,
        string DESTINO,
        USUARIO& USUARIOATUAL)
{
    //Localiza o arquivo a ser modificado
    ARQUIVO* ARQ=
        PROCURARARQUIVO(DIRETORIOATUAL,
                        ORIGEM);

    if(ARQ== nullptr)
    {
        cout<<"Erro: Arquivo nao encontrado.\n";
        return;
    }

    //CHECAGEM DE SEGURANÇA: Exige permissão 2 (Escrita) para
    if(!VALIDARPERMISSAO(*ARQ,
                         USUARIOATUAL,
                         2))
    {
        cout<<"Erro: Permissao negada.\n";
        return;
    }

    //Bloqueia a operação se o nome de destino já estiver sendo usado
    if(PROCURARARQUIVO(DIRETORIOATUAL,
                       DESTINO))
    {
        cout<<"Erro: Ja existe um arquivo com esse nome.\n";
        return;
    }

    //'mv' atua como 'rename', altera o atributo no File Control Block (FCB)
    ARQ->nome= DESTINO;

    cout<<"Arquivo renomeado com sucesso.\n";
}
