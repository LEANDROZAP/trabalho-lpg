#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#define RED      "\x1B[31m"
#define BLUE     "\x1B[34m"
#define GREEN  "\x1B[36m"
#define GRAY    "\x1B[30m"
#define RESET   "\x1B[0m"
#define CYN  "\e[0;36m"

#define MAX_LINHA 256
#define MAX_NOME 50
#define MAX_SENHA 20
#define MAX_DESCRICAO 200

typedef struct {
    char *id;
    char *tipo;
    char *descricao;
    int lotacao;
} Sala;
typedef struct {
    int idreserva;
    char *salaid;
    int dia;
    int participantes;
    int mes;
    int ano;
    int hora_inicio;
    int hora_fim;
    char *senha;
    char *descricaor;
} Reserva;


Sala *salas = NULL;
int num_salas = 0;
Reserva *reservas = NULL;
int num_reservas = 0;
int prox_id_reserva = 1;


int ehBissexto(int ano){
    if ((((ano%4)==0) && ((ano%100)!=0)) || ((ano%400)==0)){;
        return 1;
    } else{
        return 0;
    }
}

int diasNoMes(int mes, int ano) {
    switch (mes) {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            return 31;
        case 4: case 6: case 9: case 11:
            return 30;
        case 2:
            return ehBissexto(ano) ? 29 : 28;
        default:
            return 0;
    }
}

int dataValida(int dia, int mes, int ano) {
    if (ano < 1900 || ano > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > diasNoMes(mes, ano)) return 0;
    return 1;
}

void carregarSalas() {
    FILE *arquivo = fopen("salas.txt", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo das salas.\n");
        exit(1);
    }
    char linha[MAX_LINHA];
    fgets(linha, sizeof(linha), arquivo);
    while (fgets(linha, sizeof(linha), arquivo)) {
        salas = realloc( salas, (num_salas + 1) * sizeof(Sala));
        if (salas == NULL) {
            printf("Erro ao alocar memória para salas.\n");
            exit(1);
        }    
        salas[num_salas].id = malloc(MAX_LINHA);
        salas[num_salas].tipo = malloc(MAX_LINHA);
        salas[num_salas].descricao = malloc(MAX_LINHA);
        if (salas[num_salas].id == NULL || salas[num_salas].tipo == NULL || salas[num_salas].descricao == NULL) {
            printf("Erro ao alocar memória para strings de sala.\n");
            exit(1);
        }
        sscanf(linha, " %s / %[^/] / %[^/] / %d", salas[num_salas].id, salas[num_salas].tipo, salas[num_salas].descricao, &salas[num_salas].lotacao);
        num_salas++;
    }
    fclose(arquivo);
}
void salvarReservasHistorico(){
    int i;
    FILE *arquivo = fopen("reservashistorico.txt", "a");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo das reservas historico.\n");
        exit(1);
    }
 for (int i = 0; i < num_reservas; i++) {
        fprintf(arquivo, "%d, %s, %02d/%02d/%04d, %02d:00-%02d:00, %d, %s, %s\n",
                reservas[i].idreserva, reservas[i].salaid, reservas[i].dia, reservas[i].mes, reservas[i].ano,
                reservas[i].hora_inicio, reservas[i].hora_fim, reservas[i].participantes, reservas[i].senha, reservas[i].descricaor);
    }
    fclose(arquivo);
}  

void salvarReservasAtuais() {
    int i;
    FILE *arquivo = fopen("reservasatuais.txt", "w");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de reservas atuais.\n");
        exit(1);
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int diaAtual = tm.tm_mday;
    int mesAtual = tm.tm_mon + 1;
    int anoAtual = tm.tm_year + 1900;
    int horaAtual = tm.tm_hour;

    for (i = 0; i < num_reservas; i++) {
        if ((reservas[i].ano > anoAtual) || 
            (reservas[i].ano == anoAtual && reservas[i].mes > mesAtual) ||
            (reservas[i].ano == anoAtual && reservas[i].mes == mesAtual && reservas[i].dia > diaAtual) ||
            (reservas[i].ano == anoAtual && reservas[i].mes == mesAtual && reservas[i].dia == diaAtual && reservas[i].hora_fim > horaAtual)) {
            fprintf(arquivo, "%d, %s, %02d/%02d/%04d, %02d:00-%02d:00, %d, %s, %s\n",
                    reservas[i].idreserva, reservas[i].salaid, reservas[i].dia, reservas[i].mes, reservas[i].ano,
                    reservas[i].hora_inicio, reservas[i].hora_fim, reservas[i].participantes, reservas[i].senha, reservas[i].descricaor);
        }
    }
    fclose(arquivo);
}

void salvarReservaCancelada(Reserva reserva) {
    FILE *arquivo = fopen("cancelas.txt", "a");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo de reservas canceladas.\n");
        exit(1);
    }
    fprintf(arquivo, "%d, %s, %02d/%02d/%04d, %02d:00-%02d:00, %d, %s, %s / CANCELADA\n",
            reserva.idreserva, reserva.salaid, reserva.dia, reserva.mes, reserva.ano,
            reserva.hora_inicio, reserva.hora_fim, reserva.participantes, reserva.senha, reserva.descricaor);
    fclose(arquivo); 
}

void carregarReservas(){
    FILE *arquivo = fopen("reservasatuais.txt", "r");
    if (arquivo != NULL) {
        char linha[MAX_LINHA];
        while (fgets(linha, sizeof(linha), arquivo)) {
            reservas = realloc(reservas, (num_reservas + 1) * sizeof(Reserva));
            if (reservas == NULL) {
                printf("Erro ao alocar memória para reservas.\n");
                exit(1);
            }
            reservas[num_reservas].salaid = malloc(MAX_NOME);
            reservas[num_reservas].senha = malloc(MAX_SENHA);
            reservas[num_reservas].descricaor = malloc(MAX_DESCRICAO);
    if (reservas[num_reservas].salaid == NULL || reservas[num_reservas].senha == NULL || reservas[num_reservas].descricaor == NULL) {
            printf("Erro ao alocar memória para ID da sala.\n");
            exit(1);
        }
            sscanf(linha, "%d, %[^,], %d/%d/%d, %d:00-%d:00, %d, %s, %[^\n]",
                   &reservas[num_reservas].idreserva, reservas[num_reservas].salaid, &reservas[num_reservas].dia,
                   &reservas[num_reservas].mes, &reservas[num_reservas].ano, &reservas[num_reservas].hora_inicio,
                   &reservas[num_reservas].hora_fim, &reservas[num_reservas].participantes, reservas[num_reservas].senha, reservas[num_reservas].descricaor);
            if (reservas[num_reservas].idreserva >= prox_id_reserva) {
                prox_id_reserva = reservas[num_reservas].idreserva + 1;
            }
        num_reservas++;
        }
    fclose(arquivo);
    }
}

void listarSalas(){
    int i;
    printf("ID | Tipo | Descrição | Lotação\n");
    for (i=0;i<num_salas;i++){
        printf("%s, %s, %s, %d\n",salas[i].id, salas[i].tipo, salas[i].descricao, salas[i].lotacao);
    }
}
void listarReservas(){
    int i;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int diaAtual = tm.tm_mday;
    int mesAtual = tm.tm_mon + 1;
    int anoAtual = tm.tm_year + 1900;
    int horaAtual = tm.tm_hour;
    printf("ID da reserva | ID da Sala | Data | Hora | Participantes | Descricao\n");
    for (i = 0; i < num_reservas; i++) {
        if ((reservas[i].ano > anoAtual) || 
            (reservas[i].ano == anoAtual && reservas[i].mes > mesAtual) ||
            (reservas[i].ano == anoAtual && reservas[i].mes == mesAtual && reservas[i].dia > diaAtual) ||
            (reservas[i].ano == anoAtual && reservas[i].mes == mesAtual && reservas[i].dia == diaAtual && reservas[i].hora_fim > horaAtual)) {
            printf("%d |   %s |   %02d/%02d/%04d |  %02d:00-%02d:00 |   %d |  %s\n", reservas[i].idreserva, reservas[i].salaid, reservas[i].dia, reservas[i].mes, reservas[i].ano, reservas[i].hora_inicio, reservas[i].hora_fim, reservas[i].participantes,  reservas[i].descricaor);
        }
    }
}
void  listarReservasSalas(char* salaid){
    int i;
    printf("ID da reserva | ID da Sala | Data | Hora | Participantes | Descrição\n");
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int diaAtual = tm.tm_mday;
    int mesAtual = tm.tm_mon + 1;
    int anoAtual = tm.tm_year + 1900;
    int horaAtual = tm.tm_hour;

    for (i = 0; i < num_reservas; i++) {
        if (strcmp(reservas[i].salaid, salaid) == 0) {
            if ((reservas[i].ano > anoAtual) || 
                (reservas[i].ano == anoAtual && reservas[i].mes > mesAtual) ||
                (reservas[i].ano == anoAtual && reservas[i].mes == mesAtual && reservas[i].dia > diaAtual) ||
                (reservas[i].ano == anoAtual && reservas[i].mes == mesAtual && reservas[i].dia == diaAtual && reservas[i].hora_fim > horaAtual)) {
                printf("%d, %s, %02d/%02d/%04d, %02d:00-%02d:00, %d, %s\n", reservas[i].idreserva, reservas[i].salaid, reservas[i].dia, reservas[i].mes, reservas[i].ano, reservas[i].hora_inicio, reservas[i].hora_fim, reservas[i].participantes,  reservas[i].descricaor);
            }
        }
    }
}
int verificarDisponibilidade(char* sala_id, int dia, int mes, int ano, int hora_inicio, int hora_fim) {
    int i;
    for (i = 0; i < num_reservas; i++) {
        if (strcmp(reservas[i].salaid, sala_id) == 0 && reservas[i].dia == dia && reservas[i].mes == mes && reservas[i].ano == ano &&
            ((hora_inicio >= reservas[i].hora_inicio && hora_inicio < reservas[i].hora_fim) ||
             (hora_fim > reservas[i].hora_inicio && hora_fim <= reservas[i].hora_fim) ||
             (hora_inicio <= reservas[i].hora_inicio && hora_fim >= reservas[i].hora_fim))) {
            return 0;
        }
    }
    return 1;
}

void agendarSala(){
    char salaid[MAX_NOME];
    char senha[MAX_SENHA];
    char descricao[MAX_DESCRICAO];
    int dia, participantes, i, mes, ano;
    int hora_fim, hora_inicio;
    printf("Informe o ID da sala: ");
    scanf("%s", salaid);

    printf("Digite a data da reserva (DD MM AAAA): ");
    scanf("%d %d %d", &dia, &mes, &ano);

    if (!dataValida(dia, mes, ano)) {
        printf("Erro: data inválida.\n");
        return;
    }

    printf("Informe a hora de início (0-23): ");
    scanf("%d", &hora_inicio);

    printf("Informe a hora de término (0-23): ");
    scanf("%d", &hora_fim);

    printf("Informe a quantidade de participantes: ");
    scanf("%d", &participantes);

    printf("Informe uma senha para a reserva(guarde ela caso queiro alterar seu agendamento): ");
    scanf("%s", senha);

    printf("Digite uma descrição para a reserva: ");
    getchar();
    fgets(descricao, MAX_DESCRICAO, stdin);
    descricao[strcspn(descricao, "\n")] = '\0';



    int sala_encontrada = 0;
    for ( i = 0; i < num_salas; i++) {
        if (strcmp(salas[i].id, salaid) == 0) {
            sala_encontrada = 1;
            if (salas[i].lotacao < participantes) {
                printf("A lotação máxima da sala é %d. Deseja continuar mesmo assim? (s/n) s para sim, n para nao: ", salas[i].lotacao);
                char resposta;
                scanf(" %c", &resposta);
                if (resposta != 's' && resposta != 'S') {
                    return;
                }
            }
            if (!verificarDisponibilidade(salaid, dia, mes, ano, hora_inicio, hora_fim)) {
                printf("Erro: a sala já está reservada para essa data.\n");
                return;
            }
            reservas = realloc(reservas, (num_reservas + 1) * sizeof(Reserva));
            if (reservas == NULL) {
                printf("Erro ao alocar memória para reserva.\n");
                exit(1);
            }
            reservas[num_reservas].idreserva = prox_id_reserva++;
            reservas[num_reservas].salaid = strdup(salaid);
            reservas[num_reservas].dia = dia;
            reservas[num_reservas].mes = mes;
            reservas[num_reservas].ano = ano;
            reservas[num_reservas].hora_inicio = hora_inicio;
            reservas[num_reservas].hora_fim = hora_fim;
            reservas[num_reservas].participantes = participantes;
            reservas[num_reservas].senha = strdup(senha);
            reservas[num_reservas].descricaor = strdup(descricao);
            num_reservas++;
            salvarReservasHistorico();
            salvarReservasAtuais();
            printf("Sala agendada com sucesso!\n");
            return;
        }
    }
    if (!sala_encontrada) {
        printf("Erro: sala não encontrada.\n");
    }

}
void cancelarReserva(){
    int reservaid;
    int i, j;
    char senha[MAX_SENHA];
    printf("Informe o ID da reserva a ser cancelada(aperte (2) no menu para ver a id da reserva): ");
    scanf("%d", &reservaid);
    printf("Informe a senha da reserva: ");
    scanf("%s", senha);

    int encontrada = 0;
    for (i = 0; i < num_reservas; i++) {
        if (reservas[i].idreserva == reservaid && strcmp(reservas[i].senha, senha) == 0) {
            encontrada = 1;
            salvarReservaCancelada(reservas[i]);
            free(reservas[i].salaid);
            free(reservas[i].senha);
            free(reservas[i].descricaor);
            for (j = i; j < num_reservas - 1; j++) {
                reservas[j] = reservas[j + 1];
            }
            num_reservas--;
            reservas = realloc(reservas, num_reservas * sizeof(Reserva));
            if (reservas == NULL && num_reservas > 0) {
                printf("Erro ao realocar memória após cancelar reserva.\n");
                exit(1);
            }
            salvarReservasAtuais();
            printf("Reserva cancelada com sucesso!\n");
            return;
        }
    }
    if (!encontrada) {
        printf("Erro: reserva não encontrada.\n");
    }  

}

void adicionarDescricaoReserva() {
    int id_reserva;
    char nova_descricao[MAX_DESCRICAO];
    char senha[MAX_SENHA];
    int i;

    printf("Digite o ID da reserva para adicionar uma nova descrição: ");
    scanf("%d", &id_reserva);

    printf("Digite a senha da reserva: ");
    scanf("%s", senha);


     for (i = 0; i < num_reservas; i++) {
        if (reservas[i].idreserva == id_reserva && strcmp(reservas[i].senha, senha) == 0) {
            printf("Digite a nova descrição para esta reserva: ");
            getchar(); 
            fgets(nova_descricao, MAX_DESCRICAO, stdin);
            nova_descricao[strcspn(nova_descricao, "\n")] = 0;
            free(reservas[i].descricaor);
            reservas[i].descricaor = malloc(strlen(nova_descricao) + 1);
            if (reservas[i].descricaor == NULL) {
                printf("Erro ao alocar memória para a nova descrição.\n");
                exit(1);
            }
            strcpy(reservas[i].descricaor, nova_descricao);
            printf("Descrição atualizada com sucesso.\n");
            salvarReservasAtuais();
            salvarReservasHistorico();
            return;
        }
    }

    printf("Reserva não encontrada ou senha incorreta.\n");
}

void liberarMemoria() {
    int i, j;
    for ( i = 0; i < num_salas; i++) {
        free(salas[i].id);
        free(salas[i].tipo);
        free(salas[i].descricao);
    }
    free(salas);
    for (i = 0; i < num_reservas; i++) {
        free(reservas[i].salaid);
        free(reservas[i].senha);
        free(reservas[i].descricaor);
    }
    free(reservas);
    reservas = NULL;
    num_reservas = 0;
}

int main(int argc, char *argv[]){
    int i, escolha;
    carregarSalas();
    carregarReservas();
    do {        
        printf(CYN"#############################################");
        printf(RESET);
        printf(RED"\n  --MENU PRINCIPAL-- \n"RESET);
        printf("1 - Listar Salas\n");
        printf("2 - Listar Reservas\n");
        printf("3 - Reservas de uma Sala\n");
        printf("4 - Agendar Sala\n");
        printf("5 - Cancelar Reserva\n");
        printf("6 - Adicionar/alterar uma descrição/nota/um pedido a sua reserva\n");
        printf("0 - Encerrar Programa\n");
        printf(RESET);
        printf(CYN"#############################################\n");
        printf(RESET);
        printf("Escolha a sua operacao que desejar(apenas numeros): ");
        printf(RESET);
        scanf("%d", &escolha);

        switch (escolha) {
            case 1:
                listarSalas();
                break;
            case 2:
                listarReservas();
                break;
            case 3: {
                char salaid[MAX_NOME];
                printf("Qual seria o id da sala desejada?");
                scanf("%s", salaid);
                listarReservasSalas(salaid);
                break;
            }
            case 4:
                agendarSala();
                break;
            case 5:
                cancelarReserva();
                break;
            case 6:
                adicionarDescricaoReserva();
                break;
            case 0:
                liberarMemoria();
                printf("finalizando operação.\n");
                break;
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    } while (escolha != 0);
    

    return 0;
}
