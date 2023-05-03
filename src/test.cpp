#define DUMP(x) std::cout << #x << ":\t" << x << std::endl;
#include <string>
#include <sstream>
#include <random>
#include <iostream>
#include <queue>
#include <memory>
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <thread>
using std::cerr;
using std::cout;
using std::endl;
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

class ContextualState : public std::enable_shared_from_this<ContextualState>
{
private:
public:
    std::shared_ptr<ContextualState> parent_ = nullptr;
    double evaluated_score_ = 0; // 探索上で評価したスコア
    int last_action_ = -1;       // 探索木のルートノードで最初に選択した行動
    virtual ~ContextualState() {}

    // ゲームの終了判定
    virtual bool is_done() = 0;

    // ゲームの終了判定（合法だが悪い終わり方。罠にはまるなど）
    virtual bool is_dead() = 0;

    // 探索用の盤面評価をする
    virtual void evaluate_score() = 0;

    // 指定したactionでゲームを1ターン進める
    virtual void advance(const int action) = 0;

    // 現在の状況でプレイヤーが可能な行動を全て取得する
    virtual std::vector<int> legal_actions() = 0;

    // インスタンスをコピーする。
    virtual std::shared_ptr<ContextualState> clone() = 0;

    virtual std::string __str__() = 0;

    std::shared_ptr<ContextualState> cloneAdvanced(int action)
    {
        auto clone = this->clone();
        auto actions = clone->legal_actions();
        clone->advance(action);
        clone->parent_ = shared_from_this();
        clone->last_action_ = action;
        return clone;
    }
};

// 探索時のソート用に評価を比較する
bool operator<(const ContextualState &state_1, const ContextualState &state_2)
{
    return state_1.evaluated_score_ < state_2.evaluated_score_;
}
bool operator<(const std::shared_ptr<ContextualState> &state_1, const std::shared_ptr<ContextualState> &state_2)
{
    return state_1->evaluated_score_ < state_2->evaluated_score_;
}

class MazeState : public ContextualState
{
private:
    static constexpr const int dx[4] = {1, -1, 0, 0}; // 右、左、下、上への移動方向のx成分
    static constexpr const int dy[4] = {0, 0, 1, -1}; // 右、左、下、上への移動方向のy成分

    int points_[H][W] = {}; // 床のポイントを1~9で表現する
    int turn_ = 0;          // 現在のターン

public:
    Coord character_ = Coord();
    Coord trap_ = Coord();
    int game_score_ = 0; // ゲーム上で実際に得たスコア
    MazeState() {}

    // h*wの迷路を生成する。
    MazeState(const int seed)
    {
        auto mt_for_construct = std::mt19937(seed); // 盤面構築用の乱数生成器を初期化
        this->character_.y_ = mt_for_construct() % H;
        this->character_.x_ = mt_for_construct() % W;

        this->trap_.y_ = mt_for_construct() % H;
        this->trap_.x_ = mt_for_construct() % W;

        while (character_.y_ == trap_.y_ && character_.x_ == trap_.x_)
        {
            this->trap_.y_ = mt_for_construct() % H;
            this->trap_.x_ = mt_for_construct() % W;
        }

        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
            {
                if (y == character_.y_ && x == character_.x_)
                {
                    continue;
                }
                if (y == trap_.y_ && x == trap_.x_)
                {
                    continue;
                }
                this->points_[y][x] = mt_for_construct() % 10;
            }
    }

    // [どのゲームでも実装する] : ゲームの終了判定
    bool is_done() override
    {
        return this->turn_ == END_TURN;
    }

    // [どのゲームでも実装する] : ゲームの終了判定（合法だが悪い終わり方）
    bool is_dead() override
    {
        return (character_.y_ == trap_.y_ && character_.x_ == trap_.x_);
    }

    // [どのゲームでも実装する] : 探索用の盤面評価をする
    void evaluate_score() override
    {
        this->evaluated_score_ = this->game_score_; // 簡単のため、まずはゲームスコアをそのまま盤面の評価とする
    }
    // [どのゲームでも実装する] : 指定したactionでゲームを1ターン進める
    void advance(const int action) override
    {
        // std::this_thread::sleep_for(std::chrono::nanoseconds(1)); //

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
    std::vector<int> legal_actions() override
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
    std::string __str__()
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
                else if (this->trap_.y_ == h && this->trap_.x_ == w)
                {
                    ss << 'X';
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

    std::shared_ptr<ContextualState> clone() override
    {
        auto *cloned_state = new MazeState();
        *cloned_state = *this;
        return std::shared_ptr<ContextualState>(std::move(cloned_state));
    }
};

// ランダムに行動を決定する
std::vector<int> randomAction(std::shared_ptr<ContextualState> state)
{
    while (!state->is_done() && !state->is_dead())
    {
        auto legal_actions = state->legal_actions();
        int action = legal_actions[mt_for_action() % (legal_actions.size())];
        state = state->cloneAdvanced(action);
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

// ビーム幅と深さを指定してビームサーチで行動を決定する
std::vector<int> beamSearchAction_naive(std::shared_ptr<ContextualState> state, const int beam_width)
{
    std::priority_queue<std::shared_ptr<ContextualState>> now_beam;
    std::shared_ptr<ContextualState> best_state;

    now_beam.emplace(state);
    for (int t = 0;; t++)
    {
        std::priority_queue<std::shared_ptr<ContextualState>> next_beam;
        for (int i = 0; i < beam_width; i++)
        {
            if (now_beam.empty())
                break;
            std::shared_ptr<ContextualState> now_state = now_beam.top();
            if (i >= 1)
            {
                assert(now_state->parent_ != nullptr);
            }
            now_beam.pop();
            auto legal_actions = now_state->legal_actions();
            for (const auto &action : legal_actions)
            {
                auto next_state = now_state->cloneAdvanced(action);
                if (next_state->is_dead())
                {
                    continue;
                }
                next_state->evaluate_score();
                assert(next_state->parent_ != nullptr);
                next_beam.push(next_state);
            }
        }

        now_beam = next_beam;
        best_state = now_beam.top();
        if (best_state->is_done())
        {
            break;
        }
    }

    std::vector<int> actions{};
    while (best_state->parent_ != nullptr)
    {
        actions.emplace_back(best_state->last_action_);
        best_state = best_state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

// ビーム幅を指定してビームサーチで行動を決定する
std::vector<int> beamSearchAction(std::shared_ptr<ContextualState> state, const int beam_width)
{
    using ContextualStatePtr = std::shared_ptr<ContextualState>;
    std::priority_queue<ContextualStatePtr, std::vector<ContextualStatePtr>, std::greater<ContextualStatePtr>> now_beam;
    std::shared_ptr<ContextualState> best_state = nullptr;

    now_beam.emplace(state);
    for (int t = 0;; t++)
    {
        std::priority_queue<ContextualStatePtr, std::vector<ContextualStatePtr>, std::greater<ContextualStatePtr>> next_beam;

        for (int i = 0; i < beam_width; i++)
        {
            if (now_beam.empty())
                break;
            std::shared_ptr<ContextualState> now_state = now_beam.top();
            cout << "t " << t << "\tnow_score:" << now_state->evaluated_score_ << endl;

            now_beam.pop();
            auto legal_actions = now_state->legal_actions();
            for (const auto &action : legal_actions)
            {
                auto next_state = now_state->cloneAdvanced(action);
                if (next_state->is_dead())
                {
                    continue;
                }
                next_state->evaluate_score();

                if (next_beam.size() >= beam_width && next_beam.top()->evaluated_score_ >= next_state->evaluated_score_)
                {
                    continue;
                }

                assert(next_state->parent_ != nullptr);

                if (next_state->is_done())
                {
                    if (best_state == nullptr || next_state > best_state)
                    {
                        best_state = next_state;
                    }
                    continue;
                }

                next_beam.emplace(next_state);
                if (next_beam.size() > beam_width)
                {
                    next_beam.pop();
                }
            }
        }

        if (best_state != nullptr)
        {
            break;
        }
        now_beam = next_beam;
    }

    std::vector<int> actions{};
    while (best_state->parent_ != nullptr)
    {
        actions.emplace_back(best_state->last_action_);
        best_state = best_state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

// ビーム幅と深さを指定してビームサーチで行動を決定する
std::vector<int> beamSearchActionByNthElement(std::shared_ptr<ContextualState> state, const int beam_width)
{
    using ContextualStatePtr = std::shared_ptr<ContextualState>;
    std::vector<ContextualStatePtr> now_beam; // priority_queueからvectorに変更

    now_beam.emplace_back(state);
    for (int t = 0;; t++)
    {
        std::vector<ContextualStatePtr> next_beam; // priority_queueからvectorに変更

        // 常にnow_beam.size()<=beam_widthになるように後続の処理で調整し、
        // 拡張for文でnow_stateを取り出す
        for (ContextualStatePtr now_state : now_beam)
        {
            auto legal_actions = now_state->legal_actions();
            for (const auto &action : legal_actions)
            {
                ContextualStatePtr next_state = now_state->cloneAdvanced(action);
                if (next_state->is_dead())
                {
                    continue;
                }
                next_state->evaluate_score();
                next_beam.emplace_back(next_state);
            }
        }

        // ビーム幅分だけ上位のデータを残す処理をする。
        if (next_beam.size() > beam_width)
        {
            // ビーム幅分だけ上位のデータを先頭に寄せる
            std::nth_element(next_beam.begin(), next_beam.begin() + beam_width, next_beam.end(), std::greater<>());
            // ビーム幅分だけデータを切り取る
            next_beam.resize(beam_width);
        }
        now_beam = next_beam;

        if (now_beam[0]->is_done())
        {
            break;
        }
    }
    ContextualStatePtr best_state = nullptr;
    // 最も良いstateを取り出す。
    // nth_elementで処理した場合、0番目が最大であることが保証されない。
    for (ContextualStatePtr now_state : now_beam)
    {
        if (best_state == nullptr || now_state->evaluated_score_ > best_state->evaluated_score_)
        {
            best_state = now_state;
        }
    }

    std::vector<int> actions{};
    while (best_state->parent_ != nullptr)
    {
        actions.emplace_back(best_state->last_action_);
        best_state = best_state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

// ビーム幅と深さを指定してビームサーチで行動を決定する
std::vector<int> beamSearchActionMp(std::shared_ptr<ContextualState> state, const int beam_width, const int thread_n)
{
    using ContextualStatePtr = std::shared_ptr<ContextualState>;
    std::vector<std::vector<ContextualStatePtr>> now_beams(thread_n);
    std::vector<std::vector<ContextualStatePtr>> next_beams(thread_n);
    now_beams[0].emplace_back(state);
    std::vector<ContextualStatePtr> now_beam;

    for (int t = 0;; t++)
    {
        std::vector<std::thread> thds;
        for (int k = 0; k < thread_n; k++)
        {
            thds.emplace_back([&, k]
                              {
            auto &per_next_beam = next_beams[k];
            per_next_beam.clear();

            auto &per_beam = now_beams[k];
            for (ContextualStatePtr now_state : per_beam)
            {
                auto legal_actions = now_state->legal_actions();
                for (const auto &action : legal_actions)
                {
                    ContextualStatePtr next_state = now_state->cloneAdvanced(action);
                    if (next_state->is_dead())
                    {
                        continue;
                    }
                    next_state->evaluate_score();
                    per_next_beam.emplace_back(next_state);
                }
            }

            // ビーム幅分だけ上位のデータを残す処理をする。
            if (per_next_beam.size() > beam_width)
            {
                // ビーム幅分だけ上位のデータを先頭に寄せる
                std::nth_element(per_next_beam.begin(), per_next_beam.begin() + beam_width, per_next_beam.end(), std::greater<>());
                // ビーム幅分だけデータを切り取る
                per_next_beam.resize(beam_width);
            } });
        }

        // 常にnow_beam.size()<=beam_widthになるように後続の処理で調整し、
        // 拡張for文でnow_stateを取り出す
        // for (int k = 0; k < thread_n; k++)
        // {
        //     auto &per_next_beam = next_beams[k];
        //     per_next_beam.clear();

        //     auto &per_beam = now_beams[k];
        //     for (ContextualStatePtr now_state : per_beam)
        //     {
        //         auto legal_actions = now_state->legal_actions();
        //         for (const auto &action : legal_actions)
        //         {
        //             ContextualStatePtr next_state = now_state->cloneAdvanced(action);
        //             if (next_state->is_dead())
        //             {
        //                 continue;
        //             }
        //             next_state->evaluate_score();
        //             per_next_beam.emplace_back(next_state);
        //         }
        //     }

        //     // ビーム幅分だけ上位のデータを残す処理をする。
        //     if (per_next_beam.size() > beam_width)
        //     {
        //         // ビーム幅分だけ上位のデータを先頭に寄せる
        //         std::nth_element(per_next_beam.begin(), per_next_beam.begin() + beam_width, per_next_beam.end(), std::greater<>());
        //         // ビーム幅分だけデータを切り取る
        //         per_next_beam.resize(beam_width);
        //     }
        // }
        for (auto &t : thds)
        {
            t.join();
        }

        now_beam.clear();
        for (int k = 0; k < thread_n; k++)
        {
            for (auto tmp_state : next_beams[k])
            {
                now_beam.emplace_back(tmp_state);
            }
        }
        // ビーム幅分だけ上位のデータを残す処理をする。
        if (now_beam.size() > beam_width)
        {
            // ビーム幅分だけ上位のデータを先頭に寄せる
            std::nth_element(now_beam.begin(), now_beam.begin() + beam_width, now_beam.end(), std::greater<>());
            // ビーム幅分だけデータを切り取る
            now_beam.resize(beam_width);
        }

        if (now_beam[0]->is_done())
        {
            break;
        }

        for (int k = 0; k < thread_n; k++)
        {
            now_beams[k].clear();
        }

        for (int j = 0; j < now_beam.size(); j++)
        {
            int k = j % thread_n;
            now_beams[k].emplace_back(now_beam[j]);
        }
    }
    ContextualStatePtr best_state = nullptr;
    // 最も良いstateを取り出す。
    // nth_elementで処理した場合、0番目が最大であることが保証されない。
    for (ContextualStatePtr now_state : now_beam)
    {
        if (best_state == nullptr || now_state->evaluated_score_ > best_state->evaluated_score_)
        {
            best_state = now_state;
        }
    }

    std::vector<int> actions{};
    while (best_state->parent_ != nullptr)
    {
        actions.emplace_back(best_state->last_action_);
        best_state = best_state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

// シードを指定してゲーム状況を表示しながらAIにプレイさせる。
void show_game(std::shared_ptr<ContextualState> org_state, const std::vector<int> &actions)
{
    auto state = org_state->clone();
    using std::cout;
    using std::endl;
    std::string line = "######################################";
    cout << line << endl;
    cout << state->__str__() << endl;
    for (const auto &action : actions)
    {
        state->advance(action);
        cout << line << endl;
        cout << state->__str__() << endl;
    }
    cout << line << endl;
}

using AIFunction = std::function<std::vector<int>(std::shared_ptr<ContextualState>)>;
using StringAIPair = std::pair<std::string, AIFunction>;

void play_game(std::shared_ptr<ContextualState> state, StringAIPair ai)
{
    cout << "play_game start" << endl;
    auto actions = ai.second(state);
    using std::cout;
    using std::endl;
    cout << ai.first << endl;
    show_game(state, actions);
    cout << "play_game end" << endl;
}

double getAiScore(const AIFunction &ai, const int seed)
{
    auto state = std::make_shared<MazeState>(seed);
    std::vector<int> actions = ai(state);
    for (const auto &action : actions)
    {
        state->advance(action);
        state->evaluate_score();
    }
    return state->evaluated_score_;
}

int differentSeed(const StringAIPair &ai0, const StringAIPair &ai1, const int game_number)
{
    using std::cout;
    using std::endl;
    std::chrono::high_resolution_clock::time_point diff_sum;
    double score_sum = 0;
    for (int i = 0; i < game_number || game_number <= 0; i++)
    {
        auto score0 = getAiScore(ai0.second, i);
        auto score1 = getAiScore(ai1.second, i);
        if (score0 != score1)
        {
            cout << "seed " << i << ":" << ai0.first << ": " << score0 << " " << ai1.first << ": " << score1 << endl;
            return i;
        }
    }
    return -1;
}

void testAiPerformance(const StringAIPair &ai, const int game_number, const int per_game_number)
{
    using std::cout;
    using std::endl;
    std::chrono::high_resolution_clock::time_point diff_sum;
    double score_sum = 0;
    for (int i = 0; i < game_number; i++)
    {
        auto state = std::make_shared<MazeState>(i);
        auto start_time = std::chrono::high_resolution_clock::now();
        std::vector<int> actions;
        for (int j = 0; j < per_game_number; j++)
        {
            auto tmp_actions = ai.second(state);
            if (j == 0)
            {
                actions = tmp_actions;
            }
        }
        auto diff = std::chrono::high_resolution_clock::now() - start_time;
        diff_sum += diff;
        for (const auto &action : actions)
        {
            state->advance(action);
            state->evaluate_score();
        }
        score_sum += state->evaluated_score_;
    }
    double time_mean = std::chrono::duration_cast<std::chrono::milliseconds>(diff_sum.time_since_epoch()).count() / (double)(game_number);
    double score_mean = score_sum / (double)game_number;
    std::string ai_name = "                             ";
    for (int i = 0; i < std::min(ai_name.size(), ai.first.size()); i++)
    {
        ai_name[i] = ai.first[i];
    }
    cout << ai_name << "\tscore: " << score_mean << "\ttime: " << time_mean << "ms" << endl;
}

int main()
{
    using namespace std;
    // playGame(0);
    // auto a = std::shared_ptr<ContextualState>(new MazeState(0));
    // a->evaluated_score_ = 100;
    // DUMP(a->evaluated_score_);
    // auto b = std::shared_ptr<ContextualState>(a->clone());
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
    // auto actions = randomAction(state);

    int beam_width = 2;
    int thread_n = 6;
    const auto &random_ai = StringAIPair("randomAction", [&](std::shared_ptr<ContextualState> state)
                                         { return randomAction(state); });
    const auto &beam_naive_ai = StringAIPair("beamSearchAction_naive", [&](std::shared_ptr<ContextualState> state)
                                             { return beamSearchAction_naive(state, beam_width); });
    const auto &beam_ai = StringAIPair("beamSearchAction", [&](std::shared_ptr<ContextualState> state)
                                       { return beamSearchAction(state, beam_width); });
    const auto &beam_nth_ai = StringAIPair("beamSearchActionByNthElement", [&](std::shared_ptr<ContextualState> state)
                                           { return beamSearchActionByNthElement(state, beam_width); });
    const auto &beam_mp_ai = StringAIPair("beamSearchActionMp " + std::to_string(thread_n), [&](std::shared_ptr<ContextualState> state)
                                          { return beamSearchActionMp(state, beam_width, thread_n); });
    auto state = std::make_shared<MazeState>(1);
    auto actions = beam_ai.second(state);
    // cerr << "end-------" << endl;
    show_game(state, actions);
    int game_nuumber = 1;
    int per_game_nuumber = 1;
    // testAiPerformance(random_ai, game_nuumber, per_game_nuumber);
    // testAiPerformance(beam_naive_ai, game_nuumber, per_game_nuumber);
    // testAiPerformance(beam_ai, game_nuumber, per_game_nuumber);
    // testAiPerformance(beam_nth_ai, game_nuumber, per_game_nuumber);
    // testAiPerformance(beam_mp_ai, game_nuumber, per_game_nuumber);
    // int differnt_seed = differentSeed(beam_naive_ai, beam_ai, 100);
    // if (differnt_seed >= 0)
    // {
    //     auto state = std::make_shared<MazeState>(differnt_seed);
    //     play_game(state, beam_naive_ai);
    //     play_game(state, beam_ai);
    // }
    // H 5
    // W 4
    // seed 12
    // 3693
    // 8.43
    // 2X59
    // 127@
    // 8829
    // beam_width 25
    // では3ターン目で切り捨て型が負ける。しかし、dy,dxを上移動から先にするようにしたら切り捨て型も元のと同じになった。
    // 3ターン目でスコア18になるノードが多すぎて切り捨て対象になっただけ？
    // スコアを複雑化したり幅を増やせば変化するはずなので問題ないと判断できる
    return 0;
}