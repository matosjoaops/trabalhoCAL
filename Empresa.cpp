#include <iostream>
#include <fstream>
#include <cmath>
#include <time.h>
#include <chrono>
#include "Empresa.h"
#include "utils.h"

using namespace std;

Empresa::Empresa() {
    Graph<Posicao> grafo;
    this->grafo = grafo;

    readGrafo();
    readRestaurantes();
    readEncomendas();
    readEstafetas();
    readClientes();

}

void Empresa::criarEncomenda() {
    string input;

    cout<<"Indique o nome do restaurante:\n";
    getline(cin,input);

    Restaurante* restaurante = nullptr;
    vector<pair<Prato*,int>> pratos;
    Cliente* cliente = nullptr;
    int quantidadeTotal = 0;
    float custo = 0;

    for (auto rest : restaurantes)
    {
        if (rest->getNome() == input) {restaurante = rest; break;}
    }
    if (restaurante == nullptr)
    {
        cout<<"Nao ha nenhum restaurante com esse nome!\n";
        return;
    }

    do
    {
        cout<<"Indique o nome do prato que deseja:\n";
        getline(cin,input);

        string nomePrato = input;
        Prato* pratoAtual = nullptr;

        for (auto pratoRest : restaurante->getPratosDisponiveis())
        {
            if (pratoRest->getNome() == nomePrato) {pratoAtual = pratoRest; break;}
        }
        if (pratoAtual == nullptr)
        {
            cout<<"Nao ha nenhum prato com esse nome!\n";
            continue;
        }

        cout<<"Indique a quantidade que deseja:\n";
        getline(cin,input);

        int quant = stoi(input);
        quantidadeTotal+= quant;

        pratos.emplace_back(pratoAtual,quant);

        custo += pratoAtual->getPreco() * quant;

        cout<<"Pretende adicionar mais pratos (s/n)?\n";
        getline(cin,input);
        if (input != "s") break;
    } while(true);

    ///Calcular melhor estafeta
    Estafeta* estafeta = escolherEstafeta(quantidadeTotal,restaurante);

    cout<<"Indique o seu NIF:\n";
    getline(cin,input);

    for (auto cli : clientes)
    {
        if (cli->getNif() == stoi(input))
        {
            cliente = cli;
            break;
        }
    }

    cout<<"Indique a hora:\n";
    getline(cin,input);

    Hora *horaInicio = new Hora(input);

    auto caminhoTodo = calcPercurso(estafeta->getPosicao(),restaurante->getPosicao());
    auto caminhoEstafetaRestaurante = caminhoTodo;
    auto caminhoRestauranteCliente = calcPercurso(restaurante->getPosicao(),cliente->getPosicao());
    auto caminhoRestauranteCliente2 = caminhoRestauranteCliente;
    caminhoRestauranteCliente.pop_front();
    caminhoTodo.insert(caminhoTodo.end(),caminhoRestauranteCliente.begin(),caminhoRestauranteCliente.end());
    double distanciaTotal = 0;

    for (auto pos = caminhoTodo.begin(); pos != caminhoTodo.end(); pos++)
    {
        if (*pos != caminhoTodo.back())
        {
            distanciaTotal += (*pos)->getInfo().calcDist((*(pos + 1))->getInfo());
        }
    }

    float velocidade = estafeta->getTransporte()->getVelocidade();
    float tempoEmHoras = distanciaTotal/velocidade;

    int tempoMinutos = round(tempoEmHoras * 60);

    ///calcular hora do fim
    Encomenda* novaEncomenda = new Encomenda(encomendas.size()+1,pratos,restaurante,horaInicio,horaInicio->calcPassagemTempo(tempoMinutos),custo);
    novaEncomenda->setCaminhos(pair<deque<Vertex<Posicao>*>,deque<Vertex<Posicao>*>>(caminhoEstafetaRestaurante,caminhoRestauranteCliente2));
    encomendas.push_back(novaEncomenda);
    cliente->addEncomenda(novaEncomenda);
    cliente->setTotalGasto(cliente->getTotalGasto() + custo);
    estafeta->addEncomenda(novaEncomenda);
    estafeta->setLucroTotal(estafeta->getLucroTotal() + custo * 0.3);

}

void Empresa::eliminarEncomenda() {
    string input;

    cout<<"Indique o indice da encomenda a remover:\n";
    getline(cin,input);

    int index = stoi(input);

    bool found = false;
    vector<Encomenda*> encomendasAtualizadas;
    Encomenda* encomenda;
    for (auto it = encomendas.begin(); it != encomendas.end(); it++) {
        if ((*it)->getId() == index) {encomenda = (*it);encomendas.erase(it); break;}
    }
    for(Estafeta * e : estafetas){
        encomendasAtualizadas = e->getEntregasFeitas();
        for (auto it = encomendasAtualizadas.begin(); it != encomendasAtualizadas.end(); it++) {
            if (*it == encomenda) {
                encomendasAtualizadas.erase(it);
                e->setEntregasFeitas(encomendasAtualizadas);
                found  = true;
                break;
            }
        }
        if(found)
            break;
    }
    found = false;
    for(Cliente * c : clientes){
        encomendasAtualizadas = c->getEncomendasFeitas();
        for (auto it = encomendasAtualizadas.begin(); it != encomendasAtualizadas.end(); it++) {
            if (*it == encomenda) {
                encomendasAtualizadas.erase(it);
                c->setEncomendasFeitas(encomendasAtualizadas);
                found  = true;
                break;
            }
        }
        if(found)
            break;
    }

}



vector<Encomenda *> Empresa::filtrarEncomendas(int option) {
    vector<Encomenda*> result;
    string input;

    switch (option)
    {
        case 1:
        {
            result = encomendas;
            break;
        }
        case 2:
        {
            cout<<"Indique o NIF do estafeta:\n";
            getline(cin,input);
            for (auto estafeta : estafetas)
            {
                if (estafeta->getNif() == stol(input)) {result = estafeta->getEntregasFeitas(); break;}
            }
            break;
        }
        case 3:
        {
            cout<<"Indique o NIF do cliente:\n";
            getline(cin,input);
            for (auto cliente : clientes)
            {
                if (cliente->getNif() == stol(input)) {result = cliente->getEncomendasFeitas(); break;}
            }
            break;
        }
        case 4:
        {
            cout<<"Indique a hora inicial do intervalo:\n";
            getline(cin,input);

            Hora inicio(input);

            cout<<"Indique a hora inicial do intervalo:\n";
            getline(cin,input);

            Hora fim(input);

            for (auto encomenda : encomendas)
            {
                if (*encomenda->getHoraPedido() > inicio && *encomenda->getHoraEntrega() < fim) result.push_back(encomenda);
            }

            break;
        }
        default: {cout<<"Valor de opcao invalido!\n"; break;}
    }
    return result;
}

void Empresa::criarEstafeta() {
    string nome, nif, idade, idno, trpvel, trpmatricula, trpcapac;
    cout<<"Indique o nome do estafeta:\n";
    getline(cin,nome);
    cout<<"Indique o NIF do estafeta:\n";
    getline(cin,nif);
    cout<<"Indique a idade do estafeta:\n";
    getline(cin,idade);
    cout<<"Indique o ID do no em que se encontra o estafeta:\n";
    getline(cin,idno);
    cout<<"Indique a velocidade do veiculo do estafeta:\n";
    getline(cin,trpvel);
    cout<<"Indique a matricula do veiculo do estafeta:\n";
    getline(cin,trpmatricula);
    cout<<"Indique a capacidade do veiculo do estafeta:\n";
    getline(cin,trpcapac);
    Posicao posicao = grafo.getTfromId(stol(idno));
    Estafeta* estafeta = new Estafeta(nome, stol(nif), stoi(idade),stol(idno), posicao, new Transporte(trpmatricula, stof(trpvel), stoi(trpcapac)), {}, 0);
    estafetas.push_back(estafeta);
}

void Empresa::eliminarEstafeta() {
    string input;

    cout<<"Indique o NIF do estafeta a remover:\n";
    getline(cin,input);

    long nif = stol(input);

    for (auto it = estafetas.begin(); it != estafetas.end(); it++) {
        if ((*it)->getNif() == nif) {estafetas.erase(it); return;}
    }
    cout<<"Nao foi encontrado um estafeta com o NIF providenciado\n";
}

vector<Estafeta *> Empresa::filtrarEstafetas(int option) {
    vector<Estafeta*> result;
    string input;

    switch (option)
    {
        case 1:
        {
            result = estafetas;
            break;
        }
        case 2:
        {
            cout<<"Indique o nome do estafeta:\n";
            getline(cin,input);
            for (auto estafeta : estafetas)
            {
                if (estafeta->getNome().find(input) != estafeta->getNome().npos) {result.push_back(estafeta);}
            }
            break;
        }
        case 3:
        {
            cout<<"Indique o NIF do estafeta:\n";
            getline(cin,input);
            for (auto estafeta : estafetas)
            {
                if (estafeta->getNif() == stol(input)) {result.push_back(estafeta); break;}
            }
            break;
        }
        default: {cout<<"Valor de opcao invalido!\n";break;}
    }
    return result;
}


void Empresa::readEncomendas() {
    ifstream file;
    file.open("../ficheiros_texto/encomendas.txt");

    while (!file.eof())
    {
        unsigned int id;
        Prato* prat = nullptr;
        Restaurante* rest = nullptr;
        Hora *inicio = nullptr,*fim = nullptr;
        vector<pair<Prato*,int>> pratosQuants;
        float custo;

        string line;

        getline(file,line);

        id = stoi(line);

        getline(file,line);

        for (auto restaurante: restaurantes)
        {
            if (restaurante->getNome() == line)
            {
                rest = restaurante;
                getline(file,line);
                Prato* pratoAtual = nullptr;

                vector<string> nomesPratos;
                string nomePratoAtual;
                int quantidadePrato;
                auto pratos = string_split(line,',');
                for (auto prato : pratos)
                {
                    vector<string> componentes = string_split(prato,'-');
                    nomePratoAtual = componentes.at(0);
                    quantidadePrato = stoi(componentes.at(1));
                    vector<Prato*> copy = restaurante->getPratosDisponiveis();
                    for (auto prato2 : copy)
                    {
                        if (prato2->getNome() == nomePratoAtual) {pratoAtual = prato2; break;}

                    }
                    pratosQuants.emplace_back(pratoAtual,quantidadePrato);
                }
                break;
            }
        }

        if (rest == nullptr) getline(file,line);

        getline(file,line);

        inicio = new Hora(line);
        getline(file,line);
        fim = new Hora(line);

        getline(file,line);
        custo = stof(line);

        Encomenda* novaEncomenda = new Encomenda(id,pratosQuants,rest,inicio,fim,custo);
        encomendas.push_back(novaEncomenda);

        getline(file,line);
        if (line == "") break;
    }

}

vector<Cliente*> Empresa::getClientes(){
    return this->clientes;
}

vector<Estafeta*> Empresa::getEstafeta(){
    return this->estafetas;
}

vector<Encomenda*> Empresa::getEncomendas(){
    return this->encomendas;
}

vector<Restaurante*> Empresa::getRestaurantes(){
    return this->restaurantes;
}

void Empresa::criarCliente() {
    string nome, nif, idade, idno, trpvel, trpmatricula, trppeso, trpcapac;
    cout << "Indique o nome do cliente:\n";
    getline(cin, nome);
    cout << "Indique o NIF do cliente:\n";
    getline(cin, nif);
    cout << "Indique a idade do cliente:\n";
    getline(cin, idade);
    cout << "Indique o ID do no em que se encontra o cliente:\n";
    getline(cin, idno);
    Posicao posicao = grafo.getTfromId(stol(idno));
    Cliente *cliente = new Cliente(nome, stol(nif), stoi(idade), stol(idno), posicao,
    {}, 0);
    clientes.push_back(cliente);
}

void Empresa::eliminarCliente(long nif) {
    for (int i = 0; i < this->clientes.size() ; i++) {
        if(clientes.at(i)->getNif() == nif){
            clientes.erase(clientes.begin()+i);
            return;
        }
    }
    cout << "Não existe um cliente com esse NIF" << endl;
}

deque<Vertex<Posicao>*> Empresa::calcPercurso(Posicao inicio, Posicao fim) {
    return grafo.bidirectionalDijkstra(inicio, fim);
}

void Empresa::readGrafo() {
    ifstream file,file2;

    double maxX = -INF;
    double maxY = -INF;
    double minX = INF;
    double minY = INF;

    string line;
    double latitude,longitude;
    long int id,id2;


    file.open("../ficheiros_graph/full/porto_full_nodes_xy.txt");
    getline(file,line);
    while (!file.eof())
    {
        getline(file,line);
        sscanf(line.c_str(),"(%ld,%lf,%lf)",&id,&latitude,&longitude);
        if (latitude < minX) minX = latitude;
        if (latitude > maxX) maxX = latitude;
        if (longitude < minY) minY = longitude;
        if (longitude > maxY) maxY = longitude;
        Posicao novaPosicao(latitude,longitude,id);
        grafo.addVertex(novaPosicao);
        grafo.addMapPair(id);
    }
    file.close();

    grafo.setMaxX(maxX);
    grafo.setMaxY(maxY);
    grafo.setMinX(minX);
    grafo.setMinY(minY);

    long int origin,dest;
    file2.open("../ficheiros_graph/full/porto_full_edges.txt");
    getline(file2,line);

    while (!file2.eof())
    {
        getline(file2,line);
        sscanf(line.c_str(),"(%ld,%ld)",&id,&id2);
        Posicao origem = grafo.getTfromId(id);
        Posicao destino = grafo.getTfromId(id2);
        double peso = origem.calcDist(destino);
        grafo.addEdgeWithIds(id,id2,peso);
    }
    file2.close();
}

void Empresa::readRestaurantes() {
    auto mapa = readPratos();

    ifstream file;
    file.open("../ficheiros_texto/restaurantes.txt");
    string line;

    while (!file.eof())
    {
        string nomeRest;
        vector<Prato*> pratos;


        getline(file,line);
        nomeRest = line;
        getline(file,line);
        long int id = stol(line);
        Posicao posRest = grafo.getTfromId(id);
        getline(file,line);

        for (auto nomePrato : string_split(line,','))
        {
            pratos.push_back(mapa[nomePrato]);
        }

        restaurantes.push_back(new Restaurante(nomeRest,posRest,pratos));

        getline(file,line);
        if (line == "") break;
    }
    file.close();
}

map<string, Prato *> Empresa::readPratos(){
    map<string, Prato*> result;

    ifstream file;
    file.open("../ficheiros_texto/pratos.txt");

    string line;

    while (!file.eof())
    {
        string nome;
        vector<string> ingredientes;
        float preco;

        getline(file,line);
        nome = line;
        getline(file,line);
        ingredientes = string_split(line,',');
        getline(file,line);
        preco = stof(line);

        Prato* novoPrato = new Prato(nome,ingredientes,preco);
        result.insert(pair<string,Prato*>(nome,novoPrato));

        getline(file,line);
        if (line == "") break;
    }
    file.close();

    return result;
}

void Empresa::readEstafetas() {
    auto mapaMatriculaTransporte = readTransportes();

    ifstream file;
    file.open("../ficheiros_texto/estafetas.txt");

    string line;

    while (!file.eof())
    {
        string nome;
        long int nif, idno;
        int idade;
        vector<string> idEncomendas;
        vector<Encomenda*> encomendasEstafeta;
        double lucro;

        getline(file,nome);
        getline(file,line);
        nif = stol(line);
        getline(file,line);
        idade = stoi(line);
        getline(file,line);
        idno = stol(line);
        Posicao posicao = grafo.getTfromId(idno);
        getline(file,line);
        Transporte* transporte = mapaMatriculaTransporte[line];
        getline(file,line);

        idEncomendas = string_split(line,',');
        if(idEncomendas.size() == 1 && idEncomendas.at(0) == "-"){
            encomendasEstafeta = {};
            lucro = 0;
        }
        else {
            for (string id : idEncomendas) {
                for (auto encomenda : encomendas) {
                    if (encomenda->getId() == stoi(id)) {
                        encomendasEstafeta.push_back(encomenda);
                        break;
                    }
                }
            }

            getline(file, line);
            lucro = stod(line);
        }
        Estafeta* novoEstafeta = new Estafeta(nome,nif,idade,idno,posicao,transporte,encomendasEstafeta,lucro);
        estafetas.push_back(novoEstafeta);

        getline(file,line);
        if (line == "") break;
    }
    file.close();
}

map<string, Transporte *> Empresa::readTransportes() {
    map<string, Transporte*> result;

    ifstream file;
    file.open("../ficheiros_texto/transportes.txt");

    string line;

    while (!file.eof())
    {
        string matricula;
        float velocidade;
        unsigned int capacidade;

        getline(file,matricula);
        getline(file,line);
        velocidade = stof(line);
        getline(file,line);
        capacidade = stoi(line);

        Transporte* novoTransporte = new Transporte(matricula,velocidade,capacidade);

        result.insert(pair<string,Transporte*>(matricula,novoTransporte));

        getline(file,line);
        if (line == "") break;
    }
    file.close();

    return result;
}

void Empresa::readClientes() {
    ifstream file;
    file.open("../ficheiros_texto/clientes.txt");

    string line;

    while (!file.eof())
    {
        string nome;
        long int nif, idno;
        int idade;
        vector<Encomenda*> encomendasCliente;
        double totalGasto;

        getline(file,nome);
        getline(file,line);
        nif = stol(line);
        getline(file,line);
        idade = stoi(line);
        getline(file,line);
        idno = stol(line);
        Posicao posicao = grafo.getTfromId(idno);
        getline(file,line);
        auto ids = string_split(line,',');
        if(ids.size() == 1 && ids.at(0) == "-"){
            encomendasCliente = {};
        }
        else{
            for (string id : ids)
            {
                for (auto encomenda : encomendas)
                {
                    if (encomenda->getId() == stol(id))
                    {
                        encomendasCliente.push_back(encomenda);
                        break;
                    }
                }
            }
        }

        getline(file,line);
        totalGasto = stod(line);

        Cliente* novoCliente = new Cliente(nome,nif,idade,idno,posicao,encomendasCliente,totalGasto);

        clientes.push_back(novoCliente);

        getline(file,line);
        if (line == "") break;
    }
    file.close();
}

Estafeta *Empresa::escolherEstafeta(int capacidade, Restaurante *restaurante) {
    vector<Estafeta *> aux;
    Estafeta *result;
    for (Estafeta *e : estafetas) {
        if (e->getTransporte()->getCapacidade() >= capacidade)
            aux.push_back(e);
    }

    result = aux.at(0);
    double distMelhorCandidato = result->getPosicao().calcDist(restaurante->getPosicao());
    double tempoDoMelhor = distMelhorCandidato / result->getTransporte()->getVelocidade();

    for (int i = 1; i < aux.size(); i++) {
        double distEuclidiana = aux.at(i)->getPosicao().calcDist(restaurante->getPosicao());
        double tempoDoAtual = distEuclidiana / aux.at(i)->getTransporte()->getVelocidade();

        if (tempoDoAtual < tempoDoMelhor)
        {
            result = aux.at(i);
            distMelhorCandidato = result->getPosicao().calcDist(restaurante->getPosicao());
            tempoDoMelhor = distMelhorCandidato / result->getTransporte()->getVelocidade();
        }
    }
    return result;
}

void Empresa::updateClientes(){
    ofstream file;
    string encomstr;
    bool first = true;
    file.open("../ficheiros_texto/clientes.txt");
    if (file.is_open()) {
        for (auto cliente: this->getClientes()) {
            if(!first){
                file << endl << "::::::::::" << endl;
            }
            first = false;
            file << cliente->getNome() << endl;
            file << cliente->getNif() << endl;
            file << cliente->getIdade() << endl;
            file << cliente->getIdNo() << endl;
            encomstr = "";
            if(cliente->getEncomendasFeitas().empty()){
                file << "-" << endl;
                file << 0;
            }
            else{
                for (size_t i = 0; i<cliente->getEncomendasFeitas().size(); i++){
                    encomstr += to_string(cliente->getEncomendasFeitas().at(i)->getId());
                    if(i!=cliente->getEncomendasFeitas().size()-1) encomstr += ", ";
                }
                file << encomstr << endl;
                file << cliente->getTotalGasto();
            }
        }
        file.close();
        cout << "Ficheiro dos clientes atualizado!" << endl;
    }
    else{
        cout << "Erro ao aceder ao ficheiro dos clientes" << endl;
    }
}

void Empresa::updateEncomendas() {
    ofstream file;
    string pratostr;
    bool first = true;
    file.open("../ficheiros_texto/encomendas.txt");
    if (file.is_open()) {
        for (auto encomenda: this->getEncomendas()) {
            if(!first){
                file << endl << "::::::::::" << endl;
            }
            first = false;
            file << encomenda->getId() << endl;
            file << encomenda->getRestaurante()->getNome() << endl;
            pratostr = "";
            for (size_t i = 0; i<encomenda->getPratos().size(); i++){
                    pratostr += encomenda->getPratos().at(i).first->getNome() + " - " + to_string(encomenda->getPratos().at(i).second);
                    if(i!=encomenda->getPratos().size()-1) pratostr += ", ";
            }
            file << pratostr << endl;
            file << *(encomenda->getHoraPedido()) << endl;
            file << *(encomenda->getHoraEntrega()) << endl;
            file << encomenda->getCusto();
        }
        file.close();
        cout << "Ficheiro das encomendas atualizado!" << endl;
    }
    else{
        cout << "Erro ao aceder ao ficheiro dos clientes" << endl;
    }
}

void Empresa::updateEstafetas() {
    ofstream file, fileT;
    string encomstr;
    bool first = true;
    file.open("../ficheiros_texto/estafetas.txt");
    fileT.open("../ficheiros_texto/transportes.txt");
    if (file.is_open() && fileT.is_open()) {
        for (auto estafeta: this->getEstafeta()) {
            if(!first){
                file << endl << "::::::::::" << endl;
                fileT <<endl<< "::::::::::" << endl;
            }
            first = false;
            file << estafeta->getNome() << endl;
            file << estafeta->getNif() << endl;
            file << estafeta->getIdade() << endl;
            file << estafeta->getIdNo() << endl;
            file << estafeta->getTransporte()->getMatricula() << endl;
            encomstr = "";
            if(estafeta->getEntregasFeitas().empty()){
                file << "-" << endl;
                file << 0;
            }
            else{
                for (size_t i = 0; i<estafeta->getEntregasFeitas().size(); i++){
                    encomstr += to_string(estafeta->getEntregasFeitas().at(i)->getId());
                    if(i!=estafeta->getEntregasFeitas().size()-1) encomstr += ", ";
                }
                file << encomstr << endl;
                file << estafeta->getLucroTotal();
            }
            fileT << estafeta->getTransporte()->getMatricula() << endl;
            fileT << estafeta->getTransporte()->getVelocidade() << endl;
            fileT << estafeta->getTransporte()->getCapacidade();

        }
        file.close();
        cout << "Ficheiros dos estafetas e respetivos transportes atualizados!" << endl;
    }
    else{
        cout << "Erro ao aceder ao ficheiro dos estafetas ou dos transportes" << endl;
    }
}

void Empresa::mostrarCaminho() {
    long int idEncomenda;
    string input;
    cout<<"Indique o ID da encomenda em causa:\n";
    getline(cin,input);
    idEncomenda = stol(input);

    deque<Vertex<Posicao>*> caminhoEstafetaRestaurante;
    deque<Vertex<Posicao>*> caminhoRestauranteCliente;
    for (auto encomenda : encomendas)
    {
        if (encomenda->getId() == idEncomenda)
        {
            auto parCaminhos = encomenda->getCaminhos();
            caminhoEstafetaRestaurante = parCaminhos.first;
            caminhoRestauranteCliente = parCaminhos.second;
            break;
        }
    }
    if (caminhoEstafetaRestaurante.size() == 0 || caminhoRestauranteCliente.size() == 0) {cout << "Nao e possivel ver o percurso dessa encomenda.\n"; return;}


    int width = 1280;
    int height = 720;

    GraphViewer *gv = new GraphViewer(width, height, false);
    gv->createWindow(width, height);
    gv->defineVertexColor("green");
    gv->defineEdgeColor("black");
    gv->defineEdgeCurved(false);

    auto caminhoVerts = grafo.getVertexSet();
    for (auto vertex : caminhoVerts)
    {
        double yPercent, xPercent;
        xPercent = (vertex->getInfo().getLatitude() - grafo.getMinX())/(grafo.getMaxX() - grafo.getMinX()) * 0.9 + 0.05;
        yPercent = (vertex->getInfo().getLongitude() - grafo.getMinY())/(grafo.getMaxY() - grafo.getMinY()) * 0.9 + 0.05;

        gv->addNode(vertex->getInfo().getId(),(int) (xPercent*width),(int)(yPercent*height));
        gv->setVertexSize(vertex->getInfo().getId(),3);
        //gv->setVertexLabel(vertex->getInfo().getId(),to_string(vertex->getInfo().getId()));
        gv->rearrange();
    }
    vector<int> arestasDoPercurso1;
    vector<int> arestasDoPercurso2;
    int id = 0;

    for (auto vertex : caminhoVerts)
    {
        auto adjacent = vertex->getAdj();
        for (auto edge : adjacent)
        {
            gv->addEdge(id,edge.getOrig()->getInfo().getId(),edge.getDest()->getInfo().getId(),EdgeType::UNDIRECTED);

            gv->rearrange();
            bool pertenceAoCaminho = false;
            bool origemNoCaminho = false;
            bool destinoNoCaminho = false;
            for (auto vert : caminhoEstafetaRestaurante) {
                if (edge.getDest() == vert) destinoNoCaminho = true;
                if (edge.getOrig() == vert) origemNoCaminho = true;
            }

            if (destinoNoCaminho && origemNoCaminho) pertenceAoCaminho = true;
            if (pertenceAoCaminho) arestasDoPercurso1.push_back(id);

            pertenceAoCaminho = false;
            origemNoCaminho = false;
            destinoNoCaminho = false;
            for (auto vert : caminhoRestauranteCliente) {
                if (edge.getDest() == vert) destinoNoCaminho = true;
                if (edge.getOrig() == vert) origemNoCaminho = true;
            }

            if (destinoNoCaminho && origemNoCaminho) pertenceAoCaminho = true;
            if (pertenceAoCaminho) arestasDoPercurso2.push_back(id);

            id++;
        }
    }



    for (auto vert : caminhoRestauranteCliente)
    {
        if (vert == caminhoRestauranteCliente.at(0)) gv->setVertexLabel(vert->getInfo().getId(),"Restaurante");
        if (vert == caminhoRestauranteCliente.at(caminhoRestauranteCliente.size() - 1)) gv->setVertexLabel(vert->getInfo().getId(),"Cliente");
        gv->setVertexColor(vert->getInfo().getId(),"yellow");
        gv->setVertexSize(vert->getInfo().getId(),4);
    }

    for (auto vert : caminhoEstafetaRestaurante)
    {
        if (vert == caminhoEstafetaRestaurante.at(0)) gv->setVertexLabel(vert->getInfo().getId(),"Estafeta");
        gv->setVertexColor(vert->getInfo().getId(),"yellow");
        gv->setVertexSize(vert->getInfo().getId(),4);
    }

    for (auto edge : arestasDoPercurso1)
    {
        gv->setEdgeColor(edge,"orange");
        gv->setEdgeThickness(edge,2);
    }

    for (auto edge : arestasDoPercurso2)
    {
        gv->setEdgeColor(edge,"red");
        gv->setEdgeThickness(edge,2);
    }

    gv->rearrange();
    getchar();
}

void Empresa::reportarObras(bool aDecorrer) {
    string input;

    long int idOrigem, idDestino;

    cout<<"Indique o ID do no de origem:\n";
    getline(cin,input);

    idOrigem = stol(input);

    cout<<"Indique o ID do no de destino:\n";
    getline(cin,input);

    idDestino = stol(input);

    auto noOrigem = grafo.getVertexFromId(idOrigem);
    auto noDestino = grafo.getVertexFromId(idDestino);

    auto arestasAdjOrigem = noOrigem->getAdj();
    auto arestasAdjDestino = noDestino->getAdj();

    for (auto &adjEdge : arestasAdjOrigem)
    {
        if (adjEdge.getDest() == noDestino)
        {
            adjEdge.setConstruction(aDecorrer);
            break;
        }
    }

    for (auto &adjEdge : arestasAdjDestino)
    {
        if (adjEdge.getDest() == noOrigem)
        {
            adjEdge.setConstruction(aDecorrer);
            break;
        }
    }
    noOrigem->setAdj(arestasAdjOrigem);
    noDestino->setAdj(arestasAdjDestino);
}

void Empresa::analiseConectividade() {
    grafo.analyseConectivity();
}

void Empresa::analiseTemporal() {
    medirGrafo("../ficheiros_graph/pais/Aveiro/nodes_x_y_aveiro.txt","../ficheiros_graph/pais/Aveiro/edges_aveiro.txt",true);
    medirGrafo("../ficheiros_graph/pais/Aveiro/nodes_x_y_aveiro.txt","../ficheiros_graph/pais/Aveiro/edges_aveiro.txt",false);
    medirGrafo("../ficheiros_graph/pais/Braga/nodes_x_y_braga.txt","../ficheiros_graph/pais/Braga/edges_braga.txt",true);
    medirGrafo("../ficheiros_graph/pais/Braga/nodes_x_y_braga.txt","../ficheiros_graph/pais/Braga/edges_braga.txt",false);
    medirGrafo("../ficheiros_graph/pais/Coimbra/nodes_x_y_coimbra.txt","../ficheiros_graph/pais/Coimbra/edges_coimbra.txt",true);
    medirGrafo("../ficheiros_graph/pais/Coimbra/nodes_x_y_coimbra.txt","../ficheiros_graph/pais/Coimbra/edges_coimbra.txt",false);
    medirGrafo("../ficheiros_graph/pais/Ermesinde/nodes_x_y_ermesinde.txt","../ficheiros_graph/pais/Ermesinde/edges_ermesinde.txt",true);
    medirGrafo("../ficheiros_graph/pais/Ermesinde/nodes_x_y_ermesinde.txt","../ficheiros_graph/pais/Ermesinde/edges_ermesinde.txt",false);
    medirGrafo("../ficheiros_graph/pais/Fafe/nodes_x_y_fafe.txt","../ficheiros_graph/pais/Fafe/edges_fafe.txt",true);
    medirGrafo("../ficheiros_graph/pais/Fafe/nodes_x_y_fafe.txt","../ficheiros_graph/pais/Fafe/edges_fafe.txt",false);
    medirGrafo("../ficheiros_graph/pais/Gondomar/nodes_x_y_gondomar.txt","../ficheiros_graph/pais/Gondomar/edges_gondomar.txt",true);
    medirGrafo("../ficheiros_graph/pais/Gondomar/nodes_x_y_gondomar.txt","../ficheiros_graph/pais/Gondomar/edges_gondomar.txt",false);
    medirGrafo("../ficheiros_graph/pais/Lisboa/nodes_x_y_lisboa.txt","../ficheiros_graph/pais/Lisboa/edges_lisboa.txt",true);
    medirGrafo("../ficheiros_graph/pais/Lisboa/nodes_x_y_lisboa.txt","../ficheiros_graph/pais/Lisboa/edges_lisboa.txt",false);
    medirGrafo("../ficheiros_graph/pais/Maia/nodes_x_y_maia.txt","../ficheiros_graph/pais/Maia/edges_maia.txt",true);
    medirGrafo("../ficheiros_graph/pais/Maia/nodes_x_y_maia.txt","../ficheiros_graph/pais/Maia/edges_maia.txt",false);
    medirGrafo("../ficheiros_graph/pais/Porto/nodes_x_y_porto.txt","../ficheiros_graph/pais/Porto/edges_porto.txt",true);
    medirGrafo("../ficheiros_graph/pais/Porto/nodes_x_y_porto.txt","../ficheiros_graph/pais/Porto/edges_porto.txt",false);
    medirGrafo("../ficheiros_graph/pais/Viseu/nodes_x_y_viseu.txt","../ficheiros_graph/pais/Viseu/edges_viseu.txt",true);
    medirGrafo("../ficheiros_graph/pais/Viseu/nodes_x_y_viseu.txt","../ficheiros_graph/pais/Viseu/edges_viseu.txt",false);
    medirGrafo("../ficheiros_graph/espinho/espinho_full_nodes_xy.txt","../ficheiros_graph/espinho/espinho_full_edges.txt",true);
    medirGrafo("../ficheiros_graph/espinho/espinho_full_nodes_xy.txt","../ficheiros_graph/espinho/espinho_full_edges.txt",false);
    medirGrafo("../ficheiros_graph/penafiel/penafiel_full_nodes_xy.txt","../ficheiros_graph/penafiel/penafiel_full_edges.txt",true);
    medirGrafo("../ficheiros_graph/penafiel/penafiel_full_nodes_xy.txt","../ficheiros_graph/penafiel/penafiel_full_edges.txt",false);
    medirGrafo("../ficheiros_graph/full/porto_full_nodes_xy.txt","../ficheiros_graph/full/porto_full_edges.txt",true);
    medirGrafo("../ficheiros_graph/full/porto_full_nodes_xy.txt","../ficheiros_graph/full/porto_full_edges.txt",false);
    cout << "Foram gerados ficheiros csv com os tempos de execucao.\n";
}

void Empresa::medirGrafo(const string& nodePath,const string& edgePath, bool dijkstra) {
    Graph<Posicao> graph;
    long int nodeNumber, edgeNumber;
    string line;

    ifstream file,file2;
    file.open(nodePath);
    getline(file,line);
    nodeNumber = stol(line);
    while (!file.eof())
    {
        getline(file,line);
        if (line == "") break;
        line.erase(0,1);
        line.erase(line.size() - 1,1);
        auto components = string_split(line,',');
        long int id = stol(components.at(0));
        double x,y;
        x = stod(components.at(1));
        y = stod(components.at(2));
        Posicao novaPosicao(x,y,id);
        graph.addVertex(novaPosicao);
        graph.addMapPair(id);
    }
    file.close();

    file2.open(edgePath);
    getline(file2,line);
    edgeNumber = stol(line);
    while(!file2.eof())
    {
        getline(file2,line);
        if (line == "") break;
        line.erase(0,1);
        line.erase(line.size() - 1,1);
        auto components = string_split(line,',');
        long int id1,id2;
        id1 = stol(components.at(0));
        id2 = stol(components.at(1));
        auto pos1 = graph.getTfromId(id1);
        auto pos2 = graph.getTfromId(id2);
        double weight = pos1.calcDist(pos2);
        graph.addEdgeWithIds(id1,id2,weight);
    }
    file2.close();

    auto start = std::chrono::high_resolution_clock::now();
    if (dijkstra) graph.dijkstraShortestPath(graph.getTfromId(1));
    else graph.dfs();
    auto finish = std::chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::microseconds>(finish - start).count();

    ofstream csv;
    if (dijkstra) csv.open("../csv/resultados_dijkstra.csv",ofstream::out|ofstream::app);
    else csv.open("../csv/resultados_dfs.csv",ofstream::out|ofstream::app);
    csv << nodeNumber << "," << edgeNumber << "," << elapsed << endl;
    csv.close();
}









