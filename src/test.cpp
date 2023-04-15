#define DUMP(x) std::cerr << #x << ":\t" << x << std::endl;
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <queue>
#include <memory>
#include <algorithm>
// 座標を保持する
struct Coord
{
    int y_;
    int x_;
    Coord(const int y = 0, const int x = 0) : y_(y), x_(x) {}
};

std::mt19937 mt_for_action(0);                // 行動選択用の乱数生成器を初期化
using ScoreType = int64_t;                    // ゲームの評価スコアの型を決めておく。
constexpr const ScoreType INF = 1000000000LL; // あり得ないぐらい大きなスコアの例を用意しておく

constexpr const int H = 3;  // 迷路の高さ
constexpr const int W = 4;  // 迷路の幅
constexpr int END_TURN = 4; // ゲーム終了ターン

class State
{
private:
public:
    std::shared_ptr<State> parent_ = nullptr;
    double evaluated_score_ = 0; // 探索上で評価したスコア
    int last_action_ = -1;       // 探索木のルートノードで最初に選択した行動
    virtual ~State() {}

    // ゲームの終了判定
    virtual bool isDone() = 0;

    // 探索用の盤面評価をする
    virtual void evaluateScore() = 0;

    virtual void setEvaluateScore(double evaluated_score)
    {
        this->evaluated_score_ = evaluated_score;
    }

    // 指定したactionでゲームを1ターン進める
    virtual void advance(const int action) = 0;

    // 現在の状況でプレイヤーが可能な行動を全て取得する
    virtual std::vector<int> legalActions() = 0;

    // インスタンスをコピーする。
    virtual std::shared_ptr<State> clone() = 0;
};

// 探索時のソート用に評価を比較する
bool operator<(const State &state_1, const State &state_2)
{
    return state_1.evaluated_score_ < state_2.evaluated_score_;
}
bool operator<(const std::shared_ptr<State> &state_1, const std::shared_ptr<State> &state_2)
{
    return state_1->evaluated_score_ < state_2->evaluated_score_;
}

std::shared_ptr<State> cloneAdvanced(std::shared_ptr<State> state, int action)
{
    auto clone = state->clone();
    clone->advance(action);
    clone->parent_ = state;
    clone->last_action_ = action;
    return clone;
}

class MazeState : public State
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0}; // 右、左、下、上への移動方向のx成分
    static constexpr const int dy[4] = {0, 0, 1, -1}; // 右、左、下、上への移動方向のy成分

    int points_[H][W] = {}; // 床のポイントを1~9で表現する
    int turn_ = 0;          // 現在のターン

public:
    Coord character_ = Coord();
    int game_score_ = 0; // ゲーム上で実際に得たスコア
    MazeState() {}

    // h*wの迷路を生成する。
    MazeState(const int seed)
    {
        auto mt_for_construct = std::mt19937(seed); // 盤面構築用の乱数生成器を初期化
        this->character_.y_ = mt_for_construct() % H;
        this->character_.x_ = mt_for_construct() % W;

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                if (y == character_.y_ && x == character_.x_)
                {
                    continue;
                }
                this->points_[y][x] = mt_for_construct() % 10;
            }
    }

    // [どのゲームでも実装する] : ゲームの終了判定
    bool isDone() override
    {
        return this->turn_ == END_TURN;
    }
    // [どのゲームでも実装する] : 探索用の盤面評価をする
    void evaluateScore() override
    {
        this->evaluated_score_ = this->game_score_; // 簡単のため、まずはゲームスコアをそのまま盤面の評価とする
    }
    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進める
    void advance(const int action) override
    {
        this->character_.x_ += dx[action];
        this->character_.y_ += dy[action];
        auto &point = this->points_[this->character_.y_][this->character_.x_];
        if (point > 0)
        {
            this->game_score_ += point;
            point = 0;
        }
        this->turn_++;
    }

    // [どのゲームでも実装する] : 現在の状況でプレイヤーが可能な行動を全て取得する
    std::vector<int> legalActions() override
    {
        std::vector<int> actions;
        for (int action = 0; action < 4; action++)
        {
            int ty = this->character_.y_ + dy[action];
            int tx = this->character_.x_ + dx[action];
            if (ty >= 0 && ty < H && tx >= 0 && tx < W)
            {
                actions.emplace_back(action);
            }
        }
        return actions;
    }

    // [実装しなくてもよいが実装すると便利] : 現在のゲーム状況を文字列にする
    std::string toString()
    {
        std::stringstream ss;
        ss << "turn:\t" << this->turn_ << "\n";
        ss << "score:\t" << this->game_score_ << "\n";
        for (int h = 0; h < H; h++)
        {
            for (int w = 0; w < W; w++)
            {
                if (this->character_.y_ == h && this->character_.x_ == w)
                {
                    ss << '@';
                }
                else if (this->points_[h][w] > 0)
                {
                    ss << points_[h][w];
                }
                else
                {
                    ss << '.';
                }
            }
            ss << '\n';
        }

        return ss.str();
    }

    std::shared_ptr<State> clone() override
    {
        auto *cloned_state = new MazeState();
        *cloned_state = *this;
        return std::shared_ptr<State>(std::move(cloned_state));
    }
};

// ランダムに行動を決定する
std::vector<int> randomAction(std::shared_ptr<State> state)
{
    while (!state->isDone())
    {
        auto legal_actions = state->legalActions();
        int action = legal_actions[mt_for_action() % (legal_actions.size())];
        DUMP(action);
        state = cloneAdvanced(state, action);
    }
    std::vector<int> actions{};
    while (state->parent_ != nullptr)
    {
        actions.emplace_back(state->last_action_);
        state = state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

// // ビーム幅と深さを指定してビームサーチで行動を決定する
// int beamSearchAction(State *state, const int beam_width, const int beam_depth)
// {
//     std::priority_queue<State> now_beam;
//     State *best_state;

//     now_beam.emplace(*state);
//     for (int t = 0; t < beam_depth; t++)
//     {
//         std::priority_queue<State> next_beam;
//         for (int i = 0; i < beam_width; i++)
//         {
//             if (now_beam.empty())
//                 break;
//             State now_state = now_beam.top();
//             now_beam.pop();
//             auto legal_actions = now_state.legalActions();
//             for (const auto &action : legal_actions)
//             {
//                 State next_state = now_state;
//                 next_state.advance(action);
//                 next_state.evaluateScore();
//                 if (t == 0)
//                     next_state.first_action_ = action;
//                 next_beam.push(next_state);
//             }
//         }

//         now_beam = next_beam;
//         best_state = now_beam.top();

//         if (best_state.isDone())
//         {
//             break;
//         }
//     }
//     return best_state.first_action_;
// }

// // シードを指定してゲーム状況を表示しながらAIにプレイさせる。
// void playGame(const int seed)
// {
//     using std::cout;
//     using std::endl;

//     auto state = MazeState(seed);
//     cout << state.toString() << endl;
//     while (!state.isDone())
//     {
//         state.advance(randomAction(&state));
//         cout << state.toString() << endl;
//     }
// }
int main()
{
    using namespace std;
    // playGame(0);
    // auto a = std::shared_ptr<State>(new MazeState(0));
    // a->evaluated_score_ = 100;
    // DUMP(a->evaluated_score_);
    // auto b = std::shared_ptr<State>(a->clone());
    // b->parent_ = a;
    // b->evaluated_score_ = 200;
    // DUMP(a->evaluated_score_);
    // DUMP(b->evaluated_score_);
    // auto p = b;
    // while (p->parent_ != nullptr)
    // {
    //     DUMP(p->last_action_);
    //     p = p->parent_;
    // }
    auto a = std::shared_ptr<State>(new MazeState(0));
    auto actions = randomAction(a);
    cerr << "end-------" << endl;
    for (auto action : actions)
    {
        DUMP(action)
    }
    return 0;
}