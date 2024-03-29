#include "Pessoa.h"


Pessoa::Pessoa(const string &nome, long nif, int idade, long idno, Posicao posicao)
        : nome(nome), nif(nif), idade(idade),idno(idno), posicao(posicao) {}

const string &Pessoa::getNome() const {
    return nome;
}

void Pessoa::setNome(const string &nome) {
    Pessoa::nome = nome;
}

long Pessoa::getNif() const {
    return nif;
}

void Pessoa::setNif(long nif) {
    Pessoa::nif = nif;
}

int Pessoa::getIdade() const {
    return idade;
}

void Pessoa::setIdade(int idade) {
    Pessoa::idade = idade;
}

const Posicao Pessoa::getPosicao() const {
    return posicao;
}

long Pessoa::getIdNo() const {
    return idno;
}

void Pessoa::setIdNo(long idno) {
    Pessoa::idno = idno;
}

void Pessoa::setPosicao(const Posicao &posicao) {
    Pessoa::posicao = posicao;
}
