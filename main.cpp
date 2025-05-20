#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
// menor tamanho de disco possivel: 35 blocos. 1 para o boot record, 1 para o
// mapa de bits, 32 para o diretorio raiz e 1 para a área de dados.
using namespace std;
vector<char> bitmap_vetor;

// Struct responsavel por armazenar dados do boot record
typedef struct boot_record {
  unsigned char startup[3];
  unsigned short int bytes_per_block;
  unsigned char num_bitmap;
  unsigned char num_reserved_block;
  unsigned short int num_entry_raiz;
  unsigned int num_total_block;
  unsigned int size_bitmap_block;
  unsigned char fill[495];
  boot_record()
      : startup{0}, bytes_per_block(512), num_bitmap(1), num_reserved_block(1),
        num_entry_raiz(512) {}
} __attribute__((__packed__)) boot_record;

typedef struct diretorio_raiz {
  unsigned char filename[11];
  unsigned int first_block;
  unsigned int size;
  unsigned char tipo;
  unsigned char fill[12];
} __attribute__((__packed__)) diretorio_raiz;

// Classe responsavel pelo formatação
class System {
public:
  int qnt_de_blocos;
  int tam_total_disco;
  int tam_bitmap;
  string nome_arq;
  boot_record boot;
  diretorio_raiz raiz;
  int posicaoRaiz;
  int tam_rootdir_block;


  string formatador() {
     string nome_arq_disco;
    cout << endl;
    cout << "Digite o nome do disco a ser criado :" << endl;
    cin >> nome_arq_disco;
    cout << endl;
    cout << "Digite o numero de blocos desejados para o seu sistema: ";
    cin >> qnt_de_blocos;
    int valido = 0;
    while (valido == 0) {
      if (qnt_de_blocos >= 34) {
        if (qnt_de_blocos < 8388608) {
          valido = 1;
        } else {
          cout << "Digite o numero de blocos desejados valido: ";
          cin >> qnt_de_blocos;
        }
      } else {
        cout << "Digite o numero de blocos desejados valido: ";
        cin >> qnt_de_blocos;
      }
    }

    boot.num_total_block = boot.bytes_per_block * qnt_de_blocos;

    // Bitmap tamanho em blocos
    boot.size_bitmap_block = static_cast<unsigned int>(
        ceil(static_cast<float>(qnt_de_blocos) / (8 * boot.bytes_per_block)));

    memset(boot.fill, 0, sizeof(boot.fill));


    ofstream outFile(nome_arq_disco, ios::binary);
    // Verificando se o arquivo foi aberto com sucesso
    if (!outFile) {
      cerr << "Erro ao abrir o arquivo !";
    }
    // Escrevendo a estrutura no arquivo
    outFile.write(reinterpret_cast<const char *>(&boot), sizeof(boot_record));
    outFile.close();
    return nome_arq_disco;
  }

  void vetorbitmap1(string nome_arq_disco) { // lendo o bitmap da imagem e colocando no vetor
    int tambloc;
    char reservbloc;
    int bytesblock;

    const char *nome_arq_c_str = nome_arq_disco.c_str();
    FILE *fp = fopen(nome_arq_c_str, "r");
    if (fp == NULL) {
      printf("falha ao abrir imagem\n");
    }

    fseek(fp, 3, SEEK_SET); //pegar a variavel bytesblock que sera usada posteriormente
    size_t bytesRead2 = fread(&bytesblock, 2, 1, fp);
    if (bytesRead2) {
      printf("");
    }
    // cout << "bytesblock22: " << bytesblock << endl;

    fseek(fp, 6, SEEK_SET); //pegar a variavel reservbloc que sera usada posteriormente
    size_t bytesRead3 = fread(&reservbloc, 1, 1, fp);
    if (bytesRead3) {
      printf("");
    }
    // cout << "reservbloc22: " <<  static_cast<int>(reservbloc) << endl;

    fseek(fp, 9, SEEK_SET); //pegar a variavel tambloc que sera usada posteriormente
    size_t bytesRead = fread(&tambloc, 4, 1, fp);
    if (bytesRead) {
      printf("");
    }

    qnt_de_blocos = tambloc / bytesblock; //descobre a qnt_de_blocos

    boot.size_bitmap_block = static_cast<unsigned int>(ceil(static_cast<float>(qnt_de_blocos) / (8 * bytesblock))); //descobre o tamanho do bitmap em blocos

    int mapa_de_bits = reservbloc * bytesblock; //inicio do bitmap na imagem

    tam_bitmap = boot.size_bitmap_block * bytesblock; //tam bitmap em bytes
    fclose(fp);

    // Abrindo o arquivo de imagem em modo binário
    std::ifstream arquivo(nome_arq_disco, std::ios::binary);

    // Verificando se o arquivo foi aberto com sucesso
    if (!arquivo) {
      cerr << "Erro ao abrir o arquivo de imagem." << endl;
    }

    // Movendo o ponteiro do arquivo para a posição inicial do bitmap
    arquivo.seekg(mapa_de_bits, std::ios::beg);

    // Lendo os dados do bitmap e armazenando no vetor global
    char byte;
    while (arquivo.get(byte)) {
      bitmap_vetor.push_back(byte);
    }

    // Verificando se a leitura do bitmap foi bem-sucedida
    if (!arquivo.eof()) {
      cerr << "Erro ao ler o bitmap do arquivo de imagem." << endl;
    }

    // Fechando o arquivo
    arquivo.close();

    // print bitmap

    /*
    for (int i = 0; i < tam_bitmap; i++) {
      cout << "bitmap[" << i << "] = " << static_cast<int>(bitmap_vetor[i])
           << endl;
           }
           */
  }

   void escrevernobitmap(string nome_arq_disco) {

     std::fstream outFile(nome_arq_disco,
                          std::ios::binary | std::ios::in | std::ios::out);
     // Verificando se o arquivo foi aberto com sucesso
     if (!outFile) {
       cerr << "Erro ao abrir o arquivo!";
       return;
     }

     outFile.seekp(sizeof(boot_record)); // ponteiro para o inicio do bitmap
     outFile.write(bitmap_vetor.data(),
                   bitmap_vetor.size()); // Escrevendo a estrutura no arquivo

     outFile.close();

   }

  void vetorbitmap(string nome_arq_disco) {
    tam_total_disco = boot.bytes_per_block * qnt_de_blocos;
    tam_bitmap = boot.size_bitmap_block * boot.bytes_per_block;
    tam_rootdir_block = 32;

    cout << endl;
    cout << "Tamanho total do disco: " << tam_total_disco << endl;
    cout << "Tamanho do bitmap em blocos: " << boot.size_bitmap_block << endl;
    cout << "Quantos bytes em cada bloco: " << boot.bytes_per_block << endl;
    cout << "Tamanho do bitmap em bytes: " << tam_bitmap << endl << endl;

    bitmap_vetor.assign(tam_bitmap, 0);

    // colocando 1 no bitmap para os setores alem do que o usuario deseja
    for (int i = qnt_de_blocos + 1; i < tam_bitmap; i++) {
      bitmap_vetor[i] = 1;
    }

    // colocando 1 no bitmap para os setores reservados do boot record
    for (int i = 0; i < boot.num_reserved_block; i++) {
      bitmap_vetor[i] = 1;
    }

    // colocando 1 no bitmap para os setores reservados do bitmap
    for (int i = boot.num_reserved_block;
         i < (boot.num_reserved_block + boot.size_bitmap_block); i++) {
      bitmap_vetor[i] = 1;
    }

    // colocando 1 no bitmap para os setores reservados do root Dir
    for (int i = (boot.num_reserved_block + boot.size_bitmap_block);
         i <
         tam_rootdir_block + (boot.num_reserved_block + boot.size_bitmap_block);
         i++) {
      bitmap_vetor[i] = 1;
    }

    // print bitmap
    /*
    for (int i = 0; i < tam_bitmap; i++) {
      cout << "bitmap[" << i << "] = " << static_cast<int>(bitmap_vetor[i])
           << endl;
    }
    */

    escrevernobitmap(nome_arq_disco);
  }

  int encontrarBlocoLivre(const std::vector<int> &bitmap_vetor,
                          long int tamanho_arquivo, int qnt_blocos_arquivo) {
    qnt_blocos_arquivo = qnt_blocos_arquivo * 8;

    int blocosLivresContiguos = 0;
    int posicaoInicial = -1;

    for (int i = 0; i < tam_bitmap; i++) {
      // Verificar cada bit do elemento
     //cout << "bloco = " << i << endl;

      if (bitmap_vetor[i] ==
          1) { // Se todos os bits do byte estão ocupados, resete o contador de
               // blocos livres contíguos
        //cout << "bloco ocupado!! pulando para proximo bloco..." << endl << endl;
        blocosLivresContiguos = 0;
        posicaoInicial = -1;
        continue;
      }

      for (int bit = 0; bit < 8; bit++) {
        //cout << "bit = " << bit << endl;
        if (!(bitmap_vetor[i] &
              (1 << bit))) { // Verificar se o bit específico está livre (0)
          if (blocosLivresContiguos ==
              0) { // aqui é pego na variavel posicaoInicial o primeiro bit
                   // (essa condição vai ser feita novamente se por acaso nao
            // houver espaço contiguo necessario, dai ele irá realizar a
            // verificação novamente)
            posicaoInicial = i * 8 + bit;
          }
          // Incrementar contador de blocos livres contíguos
          blocosLivresContiguos++;

          // cout << "blocosLivresContiguos bit = " << blocosLivresContiguos <<
          // endl;
          // Verificar se blocos livres contíguos são suficientes
          if (blocosLivresContiguos == qnt_blocos_arquivo) {
            return posicaoInicial;
          }
        } else {
          // Se encontrar um bloco ocupado, reinicia o contador de blocos livres
          // contíguos
          blocosLivresContiguos = 0;
          posicaoInicial = -1;
        }
      }
    }

    // Se não forem encontrados blocos livres contíguos suficientes
    return -1;

  }

  void copiarDiscoSA(string nome_arq_disco) {

    cout << "Digite o nome do arquivo que deseja copiar para o Sistema de Arquivos"  << endl;
    cout << "No seguinte formato : Nome.extencao " << endl;
    cin >> nome_arq;
    const char *nome_arq_c_str = nome_arq.c_str();
    FILE *fp = fopen(nome_arq_c_str, "r");
    if (fp == NULL) {
      printf("File Not Found!\n");
    }
    fseek(fp, 0L, SEEK_END);
    long int tamanho_arquivo = ftell(fp);
    // A quantidade blocos que vai ser marcado no bitmap
    int qnt_blocos_arquivo = ceil((float)tamanho_arquivo / (float)boot.bytes_per_block);

    /**cout << "tamanho_arquivo: " << tamanho_arquivo << endl;
    cout << "qnt_blocos_arquivo: " << qnt_blocos_arquivo << endl;
    */
    // As funções abaixo, são resposavéis por manipular o BITMap
    // Se possivel retorna irá retorna uma posição valida

    int achou = encontrarBlocoLivre(std::vector<int>(bitmap_vetor.begin(), bitmap_vetor.end()),tamanho_arquivo,qnt_blocos_arquivo);

    int posicaoInicialVET = achou / 8; // posição i do vetor onde deve ser gravado o arquivo

    if (achou != -1) { // se blocoLivre diff de -1 então existe bloco livre
     // cout << "DEVE SER ALOCADO NA POSIÇÃO: BITMAP[" << posicaoInicialVET << "]" << endl;
     // cout << "NO BIT: " << achou << " (contando do 0) " << endl;
    } else { // se blocoLivre == -1 então NÃO existe bloco livre
      cout << "Nenhum bloco livre encontrado." << endl;
    }

    //alocar
    //cout << "Alocando no BITMAP..." << endl;
    for (int i = 0; i < qnt_blocos_arquivo; i++) {
      bitmap_vetor[posicaoInicialVET+i] = 1;
    }
    //cout << "Alocado no BITMAP com sucesso!!" << endl;

    escrevernobitmap(nome_arq_disco);

    //Fim da manipulação do BITMAP
    fclose(fp);
    // Abertura do arquivo do Disco.
    FILE *diretorio = fopen(nome_arq_disco.c_str(),"rb"); 
    if (diretorio == nullptr) {
      cout << "Erro ao abrir o arquivo do disco" << endl;
    }
    if (fseek(diretorio, 0, SEEK_SET) != 0) {
      perror("Erro ao posicionar o cursor no arquivo");
    }
    // Escrita do diretório raiz no arquivo do disco
    if (fread(&boot, sizeof(boot_record), 1, diretorio) != 1) {
      perror("Erro ao ler os dados do arquivo boot");
    }

    int tam_mapa_bits_blocos = ceil((float)boot.num_total_block / (float)(8 * boot.bytes_per_block));
    int tam_mapa_bits_bytes = tam_mapa_bits_blocos * boot.bytes_per_block;
    posicaoRaiz = (sizeof(boot_record) + tam_mapa_bits_bytes);
    // Salva a posicao inicial do diretório raiz
    int posicaoRaiz_inicial = posicaoRaiz; 
    int controle = 1;

    while (controle == 1) {
      if (fseek(diretorio, posicaoRaiz, SEEK_SET) != 0) {
        perror("Erro ao posicionar o cursor no arquivo");
      }
      // Lendo os dados do arquivo para o buffer usando fread
      if (fread(&raiz, sizeof(diretorio_raiz), 1, diretorio) != 1) {
         // perror("Erro ao ler o arquivo IRR");
        break;
      }
      if (raiz.filename[0] == 0x00 || raiz.tipo == 0xE5) { // Posicao vazia ou excluida
        fclose(diretorio);
        //cout << " Posição do Diretorio Raiz : " << posicaoRaiz << endl;
        controle = 0;

      } else {
        posicaoRaiz += 32; // pula para o proximo posicao do diretorio raiz

      }
    }
    // Necessario usar outra função para manipular o disco, devido a erros ocasionados com fread
    ofstream file(nome_arq_disco, ios::binary | ios::in |ios::out); // Abre o arquivo em modo binário
    if (!file.is_open()) {
      cout << "Erro ao abrir o arquivo" << endl;
    }
    // especifica a posicao para escrever o root dir
    file.seekp(posicaoRaiz);
    // Pode escrever nessa posicao
    copy(nome_arq.begin(), nome_arq.end(), raiz.filename);
    raiz.filename[nome_arq.size()] = '\0'; // Adicionar o caractere nulo
    raiz.first_block = posicaoInicialVET; 
    raiz.size = tamanho_arquivo;
    raiz.tipo = 1;
    memset(raiz.fill, 0, sizeof(raiz.fill));
    // Defina a posição onde deseja começar a escrever
    file.seekp(posicaoRaiz);
    // Escrevendo a estrutura no arquivo
    file.write(reinterpret_cast<const char *>(&raiz), sizeof(diretorio_raiz));

    // ARMAZENANDO OS CONTEUDO
    ifstream arquivo(nome_arq); // Abre o arquivo para leitura
    if (!arquivo) {
      std::cerr << "Erro ao abrir o arquivo!" << endl;
    }

    string linha;
    string dados_block;
    int tam_dir_raiz_bytes =  boot.bytes_per_block * 32; // Limite maximo de entrada do dir raiz
    int posicaoDados = (tam_dir_raiz_bytes + posicaoRaiz_inicial) ;
    posicaoDados += (raiz.first_block -33) *  boot.bytes_per_block; // Ira colocar na posicao correta livre
    cout << endl;
    cout << "O arquivo " << raiz.filename << " com o tamanho " << raiz.size << endl;   
    cout << "A localizacao do diretorio raiz é " << posicaoRaiz  << endl;
    cout << "O inicio da area de dados do arquivo é  " << posicaoDados  << " ocupando ao todo "<< qnt_blocos_arquivo  << " blocos " << endl;
    cout << endl;

    // Loop de leitura até encontrar o final do arquivo
    while (getline(arquivo, linha)) {
      dados_block.append(linha);
    }

    for (int i = 0; i < qnt_blocos_arquivo; ++i) {
      file.seekp(posicaoDados);
      // file.write(dados_block.c_str(), dados_block.size());
      if (dados_block.size() >= boot.bytes_per_block) {
        // Extrai o primeiro Bloco
        string um_bloco_dados = dados_block.substr(0, boot.bytes_per_block);
        // Atualiza a string
        dados_block = dados_block.substr(boot.bytes_per_block);
        file.write(um_bloco_dados.c_str(), um_bloco_dados.size());

      } else {
        file.write(dados_block.c_str(), dados_block.size());
      }
      posicaoDados += boot.bytes_per_block;
    }

    // Fechando o arquivo
     file.close();    
     arquivo.close();
  }

  void ExclusaoArquivo(string nome_arq_disco) {

    cout << "Digite o nome do arquivo que deseja excluir do Sistema de Arquivos"  << endl;
    cout << "No seguinte formato : Nome.extencao " << endl;
    cin >> nome_arq;

    ifstream arquivo(nome_arq_disco,ios::binary); // Abre o arquivo em modo binário para leitura
    if (!arquivo.is_open()) {
      cout << "Erro ao abrir o arquivo." << endl;
      return;
    }
    arquivo.read(reinterpret_cast<char *>(&boot), sizeof(boot_record));

    int tam_mapa_bits_blocos = ceil((float)boot.num_total_block / (float)(8 * boot.bytes_per_block));
    int tam_mapa_bits_bytes = tam_mapa_bits_blocos * boot.bytes_per_block;

    posicaoRaiz = (sizeof(boot_record) + tam_mapa_bits_bytes); 

    int controle = 1;
    while (controle == 1) {
      // Posiciona o cursor no início do diretório raiz
      arquivo.seekg(posicaoRaiz); // Posiciona o cursor no início do diretório raiz
      arquivo.read(reinterpret_cast<char *>(&raiz), sizeof(diretorio_raiz));

      if (raiz.filename[0] == 0x00) { // se for 00 indica que acabou a entrada de arquivos
        controle = 0;
        cout << "Arquivo solicitado não esta armazenado no sistema de arquivo " << endl;

        // Verifica se é o arquivo desejado
      } else if (strcmp(reinterpret_cast<const char *>(raiz.filename),nome_arq.c_str()) == 0) {
        if(raiz.tipo == 0xE5){
          controle = 0;
          cout << "Arquivo solicitado não esta armazenado no sistema de arquivo " << endl;
        }else{        

          // Primeiro será calculado a quantidade de blocos que o arquivo ocupa no mapa de bits
          int qnt_blocos_arquivo = ceil((float)raiz.size / (float)boot.bytes_per_block);         
          for (int i = 0; i < qnt_blocos_arquivo; i++) { //percorre o bitmap nas posições do arquivo
            bitmap_vetor[raiz.first_block + i] = 0; //coloca 0 no bitmap
          }

          escrevernobitmap(nome_arq_disco); //escreve na imagem

          raiz.tipo = 0xE5;
          // Fecha o ponteiro de leitura
          arquivo.close();
          // Abre o Ponteiro para escrita
          ofstream arquivo(nome_arq_disco, ios::binary | ios::in | ios::out);
          arquivo.seekp(posicaoRaiz); 

          arquivo.write(reinterpret_cast<char *>(&raiz), sizeof(diretorio_raiz));
          controle = 0;
          cout << "O arquivo " << raiz.filename << " foi excluido com sucesso !" << endl;
        }

      } else {
        posicaoRaiz += 32; // pula para o proximo arquivo
      }
    }
    arquivo.close();
  }

 void ListagemArquivo(string nome_arq_disco) {

   ifstream arquivo(nome_arq_disco,ios::binary); // Abre o arquivo em modo binário para leitura
   if (!arquivo.is_open()) {
     cout << "Erro ao abrir o arquivo." << endl;
     return;
   }
   arquivo.read(reinterpret_cast<char *>(&boot), sizeof(boot_record));

   int tam_mapa_bits_blocos = ceil((float)boot.num_total_block / (float)(8 * boot.bytes_per_block));
   int tam_mapa_bits_bytes = tam_mapa_bits_blocos * boot.bytes_per_block;
   // Calcula a posição do diretório raiz
   posicaoRaiz = (sizeof(boot_record) + tam_mapa_bits_bytes) ;

   int controle = 1;

   arquivo.seekg(0, ios::end); 
   streampos tamanhoArquivo = arquivo.tellg(); 
   if (tamanhoArquivo < posicaoRaiz + sizeof(diretorio_raiz)) {
    cout << endl;
        cout << "Disco ainda nao foi preenchido" << endl;
        cout << "Volte aqui depois de adicionar um ou mais arquivos" << endl;
        cout << endl;
        controle = 0;
   } 

   while (controle == 1) {
     // Posiciona o cursor no início do diretório raiz
     arquivo.seekg(posicaoRaiz); // Posiciona o cursor no início do diretório raiz
     arquivo.read(reinterpret_cast<char *>(&raiz), sizeof(diretorio_raiz));


       if (raiz.filename[0] == 0x00 ) { // se for 00 indica que acabou a entrada de arquivos
              controle = 0;
        } else  if(raiz.tipo != 0xE5){
          cout << endl;
          cout << " Nome arquivo :  " << raiz.filename << endl;
          cout << " Tamanho do arquivo : " << raiz.size << endl;
            if (raiz.tipo == 1){
              cout << " Tipo arquivo : 1 (Arquivo) " << endl;
            } else{
           cout << " Tipo arquivo : Indefinido"  << endl;
         }        
         cout << " Primeiro bloco do arquivo : " << raiz.first_block << endl;
         cout << endl;
         posicaoRaiz += 32; // pula para o proximo arquivo      a ser lido         
          } else {
            posicaoRaiz += 32; // pula para o proximo arquivo
          }
      }

   arquivo.close(); 
 }

  void CopiaSAparaSO(string nome_arq_disco) {
    cout << endl;
    cout << "Digite o nome do arquivo que deseja copiar do Sistema de Arquivos para fora"  << endl;
    cout << "No seguinte formato : Nome.extencao " << endl;
    cin >> nome_arq;

    ifstream arquivo(nome_arq_disco, ios::binary);
    arquivo.read(reinterpret_cast<char *>(&boot), sizeof(boot_record));
    char *um_bloco_dados = static_cast<char *>(malloc(boot.bytes_per_block));
    int tam_mapa_bits_blocos = ceil((float)boot.num_total_block / (float)(8 * boot.bytes_per_block));
    int tam_mapa_bits_bytes = tam_mapa_bits_blocos * boot.bytes_per_block;
    // Calcula a posição do diretório raiz
    posicaoRaiz = (sizeof(boot_record) + tam_mapa_bits_bytes);
    int posicaoRaiz_inicial = posicaoRaiz;
    int controle = 1;
    while (controle == 1) {
      // Posiciona o cursor no início do diretório raiz
      arquivo.seekg(
          posicaoRaiz); // Posiciona o cursor no início do diretório raiz
      arquivo.read(reinterpret_cast<char *>(&raiz), sizeof(diretorio_raiz));
      if (raiz.filename[0] == 0x00) {

        controle = 0;
        cout << "Arquivo solicitado não esta armazenado no sistema de arquivo "
             << endl;
      } else if (strcmp(reinterpret_cast<const char *>(raiz.filename),
                        nome_arq.c_str()) == 0) {
        if (raiz.tipo == 0xE5) {
          controle = 0;
          cout << "Arquivo solicitado não esta armazenado no sistema de arquivo "
               << endl;
        } else {
          int tam_dir_raiz_bytes = boot.bytes_per_block * 32;
          int posicaoDados = (tam_dir_raiz_bytes + posicaoRaiz_inicial);
          posicaoDados += (raiz.first_block-33) * boot.bytes_per_block;
          int qnt_blocos_arquivo = ceil((float)raiz.size / (float)boot.bytes_per_block);
          // Calcula a quantidade exata do ultimo bloco para evitar lixo.
          int tam_ultimo_bloco =  raiz.size % boot.bytes_per_block;
          ofstream saida(nome_arq);
          for (int i = 0; i < qnt_blocos_arquivo; ++i) {           
            if (i == qnt_blocos_arquivo - 1) {              
              if (tam_ultimo_bloco != 0){
                arquivo.seekg(posicaoDados); 
                char *ultimo_bloco_dados = static_cast<char *>(malloc(tam_ultimo_bloco));
                arquivo.read(reinterpret_cast<char *>(ultimo_bloco_dados),tam_ultimo_bloco);
                saida.seekp(i * boot.bytes_per_block);
                 saida << ultimo_bloco_dados;
              }

            } else {
              arquivo.seekg(posicaoDados); // Posiciona o cursor de escrita 
              posicaoDados += boot.bytes_per_block;
              arquivo.read(reinterpret_cast<char *>(um_bloco_dados), boot.bytes_per_block);
              saida.seekp(i * boot.bytes_per_block); 
             // saida.write(reinterpret_cast<char *>(um_bloco_dados),sizeof(i * boot.bytes_per_block));
              saida << um_bloco_dados;
            }
          }
          saida.close();
          controle = 0;
        }

      } else {
        posicaoRaiz += 32; // pula para o proximo arquivo
      }
    }

    arquivo.close();
  }
};

// ***********************  Main *********************************/
int main() {
System system_obj;
int opcao;
string nome_arq_disco;
int operacao = 0;
int disco_existe = 0;
int controle = 0;
while (controle == 0){
  if (disco_existe == 0){
    cout << "Voce deseja criar um novo disco ou abrir um existente? " << endl;
    cout << "Digite :  0 - Para novo  | 1 - Para existente" << endl;
    cin >> opcao;
    if (opcao == 0) {

      nome_arq_disco = system_obj.formatador(); // Chamando o método formatador
      system_obj.vetorbitmap(nome_arq_disco);
    } else {
      cout << "Digite o nome do disco que deseja abrir: ";
      cin >> nome_arq_disco;
      system_obj.vetorbitmap1(nome_arq_disco);
    }
    disco_existe = 1;
    cout << endl;
    cout << "O Sistema de Arquivos irá manipular o disco : " << nome_arq_disco << endl;
  }else{
    cout << "|-------------------------------------------------------|" << endl;
    cout << "|----------------  O que deseja fazer ?  -------------- |" << endl;
    cout << "| Digite 1 - Para criar um novo disco                   |" << endl;
    cout << "| Digite 2 - Para copiar um arquivo de fora para o disco|" << endl;
    cout << "| Digite 3 - Para copiar um arquivo do disco para fora  |" << endl;
    cout << "| Digite 4 - Para excluir um arquivo do disco           |" << endl;
    cout << "| Digite 5 - Para listar todos arquivos do disco        |" << endl;
    cout << "| Digite 6 - Para sair do sistema                       |" << endl;
    cout << "|-------------------------------------------------------|" << endl;
    cin >> operacao;
    cout << endl;

    switch (operacao) {
        case 1:
            nome_arq_disco = system_obj.formatador(); 
            system_obj.vetorbitmap(nome_arq_disco);
            break;
        case 2:
            system_obj.copiarDiscoSA(nome_arq_disco); 
            break;
        case 3:
            system_obj.CopiaSAparaSO(nome_arq_disco); 
            break;
        case 4:
            system_obj.ExclusaoArquivo(nome_arq_disco);
            break;
        case 5:
            system_obj.ListagemArquivo(nome_arq_disco);
        break;
        case 6:
            controle = 1;
          break;            
        default:
            break;
      }

  }

}
}