#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include <string>
#include <vector>

using namespace std;


const int LER_DONO = 0400;
const int ESCREVER_DONO = 0200;
const int EXECUTAR_DONO = 0100;
const int LER_OUTROS = 0004;
const int ESCREVER_OUTROS = 0002;
const int EXECUTAR_OUTROS = 0001;
const int TAMANHODODISCO = 1024;

extern char DISCOSIMULADO[TAMANHODODISCO];
extern int PROXIMOBLOCOLIVRE;

struct GRUPO;

struct USUARIO
{
    string nome;
    int id;
    GRUPO* grupo{};
    string permissao;
};

struct GRUPO
{
    string nome = "Sem grupo";
    int id = 0;
    string permissao = "---";
    vector<USUARIO> listademembros;
};

enum TIPODOARQUIVO { texto, dados, programa };

struct ARQUIVO
{
    string nome;
    string conteudo;
    int tamanho;
    TIPODOARQUIVO tipo;
    int id;
    int iddodono;
    int iddogrupo;
    int permissaobitmask;
    int bloco_inicial;
};

struct DIRETORIO
{
    string nome;
    DIRETORIO* pai{};
    vector<DIRETORIO*> subdiretorios;
    vector<ARQUIVO> listaarquivos;
};


int CONVERTERSTRINGPARABITMASK(string PERMISSAO);
bool VALIDARPERMISSAO(ARQUIVO& ARQ, USUARIO& USUARIOATUAL, int TIPOOPERACAO);
bool ALOCARCONTEUDONODISCO(ARQUIVO& ARQ);

ARQUIVO* PROCURARARQUIVO(DIRETORIO* DIRETORIOATUAL, string NOME);

void CP(DIRETORIO* DIRETORIOATUAL,
        string ORIGEM,
        string DESTINO,
        USUARIO& USUARIOATUAL);

void MV(DIRETORIO* DIRETORIOATUAL,
        string ORIGEM,
        string DESTINO,
        USUARIO& USUARIOATUAL);

#endif
