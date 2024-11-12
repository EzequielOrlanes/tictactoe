#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <random>
using namespace std;
// Classe TicTacToe
class TicTacToe
{
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex;                   // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv;          // Variável de condição para alternância de turnos
    char current_player;                      // Jogador atual ('X' ou 'O')
    bool game_over;                           // Estado do jogo
    char winner;                              // Vencedor do jogo ('X', 'O' ou 'D' para empate)
public:
    TicTacToe() : current_player('X'), game_over(false), winner(' '){
        // Inicializar o tabuleiro com espaços vazios
        for (auto &row : board)
            row.fill(' ');
    }
    void display_board(){
        std::lock_guard<std::mutex> lock(board_mutex);
        for (const auto &row : board){
            for (char cell : row)
            cout << cell << " ";
            cout << std::endl;
        }
    }
    bool make_move(char player, int row, int col){
        std::unique_lock<std::mutex> lock(board_mutex);
        // Espera até que seja o turno do jogador atual ou o jogo tenha terminado
        turn_cv.wait(lock, [this, player] { 
            return current_player == player || game_over; 
        });
        if (game_over) return false;

        if (board[row][col] == ' '){
            board[row][col] = player;
            if (check_win(player)){
                winner = player;
                game_over = true;
            }
            else if (check_draw()){
                winner = 'D'; // Empate
                game_over = true;
            }
            display_board();
            current_player = (current_player == 'X') ? 'O' : 'X';
            // display_board();
            turn_cv.notify_all(); // Notifica as threads para verificar o próximo turno
            return true;
        }
        return false;
    }
    
    bool check_win(char player)
    {
        for (int i = 0; i < 3; ++i){
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) || 
                (board[0][i] == player && board[1][i] == player && board[2][i] == player))
                return true;
        }
        return (board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
               (board[0][2] == player && board[1][1] == player && board[2][0] == player);
    }

    bool check_draw(){
        for (const auto &row : board)
            for (char cell : row)
                if (cell == ' ') return false;
        return true;
    }
    bool is_game_over() const { return game_over; }
    char get_winner() const { return winner; }
};

// Classe Player
class Player
{
private:
    TicTacToe &game;      // Referência para o jogo
    char symbol;          // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador
    std::mt19937 gen;     // Gerador de números aleatórios

public:
    Player(TicTacToe &g, char s, std::string strat)
        : game(g), symbol(s), strategy(strat), gen(std::random_device{}()) {}
    void play()
    {
        if (strategy == "sequential")
            play_sequential();
        else if (strategy == "random")
            play_random();
    }

private:
    void play_sequential(){
        for (int i = 0; i < 3 && !game.is_game_over(); ++i)
            for (int j = 0; j < 3 && !game.is_game_over(); ++j)
                game.make_move(symbol, i, j);
    }
    void play_random(){
        std::uniform_int_distribution<> dist(0, 2);
        while (!game.is_game_over()){
            int row = dist(gen);
            int col = dist(gen);
            game.make_move(symbol, row, col);
        }
    }
};

// Função principal
int main(){
    TicTacToe game;
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");
    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);
    t1.join();
    t2.join();
    game.display_board();
    char winner = game.get_winner();
    if (winner == 'D')
        cout << "The game is a draw!" << std::endl;
    else
        cout << "Player " << winner << " wins!" << std::endl;
    return 0;
}
