# ====================
# score4
# ====================

import random
import math
import collections

MARKS1 = {-1:' ✕ ', 1:' ○ '}#辞書形式で0のプレイヤーは✕、1のプレイヤーは○と定義する
MARKS2 = {1:' ✕ ', -1:' ○ '}

lines = [
            [0, 4, 8, 12],
            [1, 5, 9, 13],
            [2, 6, 10, 14],
            [3, 7, 11, 15],
            [0, 1, 2, 3],
            [4, 5, 6, 7],
            [8, 9, 10, 11],
            [12, 13, 14, 15],
            [3, 6, 9, 12],
            [0, 5, 10, 15]
        ]

#各面で切った時の座標のデータ
s_co = [
            [0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60],
            [1, 5, 9, 13, 17, 21, 25, 29, 33, 37, 41, 45, 49, 53, 57, 61],
            [2, 6, 10, 14, 18, 22, 26, 30, 34, 38, 42, 46, 50, 54, 58, 62],
            [3, 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63],
            [0, 1, 2, 3, 16, 17, 18, 19, 32, 33, 34, 35, 48, 49, 50, 51],
            [4, 5, 6, 7, 20, 21, 22, 23, 36, 37, 38, 39, 52, 53, 54, 55],
            [8, 9, 10, 11, 24, 25, 26, 27, 40, 41, 42, 43, 56, 57, 58, 59],
            [12, 13, 14, 15, 28, 29, 30, 31, 44, 45, 46, 47, 60, 61, 62, 63],
            [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
            [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31],
            [32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47],
            [48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63],
            [3, 6, 9, 12, 19, 22, 25, 28, 35, 38, 41, 44, 51, 54, 57, 60],
            [0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63]
        ]

# ゲーム状態
class State:
    # 初期化
    def __init__(self, pieces=None, enemy_pieces=None):
        # 石の配置
        self.pieces = pieces if pieces != None else [0] * 64
        self.enemy_pieces = enemy_pieces if enemy_pieces != None else [0] * 64

    # 石の数の取得
    def piece_count(self, pieces):
        count = 0
        for i in pieces:
            if i == 1:
                count +=  1
        return count

    def surface(self, se_or_en):#self or enermy
        p = [self.pieces, self.enemy_pieces]
        s = p[se_or_en]
        z1 = s[0:16]
        z2 = s[16:32]
        z3 = s[32:48]
        z4 = s[48:64]
        e = [z1,z2,z3,z4]
        x1, x2, x3, x4, y1, y2, y3, y4, xy1, xy2 = [], [], [], [], [], [], [], [], [], []
        for i in range(0, len(e)):
            for j in range(0, len(e)):
                x1.append(e[i][4*j])
                x2.append(e[i][4*j+1])
                x3.append(e[i][4*j+2])
                x4.append(e[i][4*j+3])
                y1.append(e[i][j])
                y2.append(e[i][j+4])
                y3.append(e[i][j+8])
                y4.append(e[i][j+12])
                xy1.append(e[i][(j+1)*3])
                xy2.append(e[i][j*5])
        s = [x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4, xy1, xy2]
        return s

    #敵が勝てば負け
    def is_lose(self):
        s = State.surface(self,1)#1を指定するとenermyの石を取ってくる。
        sur = ['x1', 'x2', 'x3', 'x4', 'y1', 'y2', 'y3', 'y4', 'z1', 'z2', 'z3', 'z4', 'xy1', 'xy2']

        for i in range (0,len(s)):
            for j in range(0,len(lines)):
                [a,b,c,d] = lines[j]
                # print(i, j, [a,b,c,d],sur[i])
                if (s[i][a] == s[i][b] and s[i][a] == s[i][c] and \
                        s[i][a] == s[i][d]):
                    if s[i][a] != 0:
                        return True
        return False

    # 引き分けかどうか
    def is_draw(self):
        return self.piece_count(self.pieces) + self.piece_count(self.enemy_pieces) == 64

    # ゲーム終了かどうか
    def is_done(self):
        return self.is_lose() or self.is_draw()

    def next(self, action):
        pieces = self.pieces.copy()
        for j in range(4):# 0, 1, 2, 3
            if self.pieces[action+j*16] == 0 and self.enemy_pieces[action+j*16] == 0:
                pieces[action+j*16] = 1
                break
        return State(self.enemy_pieces, pieces)
    
    def unmove(self, action):
        pieces = self.pieces.copy()
        for j in range(4):# 0, 1, 2, 3
            if self.pieces[action+j*16] != 0 or self.enemy_pieces[action+j*16] != 0:
                pieces[action+j*16] = 0
                break
        return State(self.enemy_pieces, pieces)

    # 合法手のリストの取得
    def legal_actions(self):
        actions = []
        for i in range(16):
            if self.pieces[i + 16*3] == 0 and self.enemy_pieces[i + 16*3] == 0:
                actions.append(i)
        return actions

    # 先手かどうか
    def is_first_player(self):
        return self.piece_count(self.pieces) == self.piece_count(self.enemy_pieces)

    def inverse_convert(self, idx):
        l = [0,0,0]#100*a + 10*b + cにする
        for i in range(0,4):#ここのiはzの値をみてる
            if idx < 16*(i+1) and idx >= 16*i:
                #まずは1桁目から
                if idx%4 == 0:
                    l[0] = 1
                elif idx%4 == 1:
                    l[0] = 2
                elif idx%4 == 2:
                    l[0] = 3
                elif idx%4 == 3:
                    l[0] = 4

                #2桁目
                if idx-16*i == 0 or idx-16*i == 1 or idx-16*i == 2 or idx-16*i == 3:
                    l[1] = 1
                elif idx-16*i == 4 or idx-16*i == 5 or idx-16*i == 6 or idx-16*i == 7:
                    l[1] = 2
                elif idx-16*i == idx-16*i == 8 or idx-16*i == 9 or idx-16*i == 10 or idx-16*i == 11:
                    l[1] = 3
                elif idx-16*i == idx-16*i == 12 or idx-16*i == 13 or idx-16*i == 14 or idx-16*i == 15:
                    l[1] = 4

                #3桁目
                l[2] = i+1

        idx = 100*l[0] + 10*l[1] + l[2]
        return idx

    #3桁で入力された数字を0~63に変換する
    def convert(self, idx):
        s = str(idx)
        # １文字ずつ数値化し配列にする。
        idx_list = list(map(int, s))
        if idx%10 == 1:
            idx = 4*(idx_list[1]-1)+idx_list[0]-1+0
            return idx
        elif idx%10 == 2:
            idx = 4*(idx_list[1]-1)+idx_list[0]-1+16
            return idx
        elif idx%10 == 3:
            idx = 4*(idx_list[1]-1)+idx_list[0]-1+32
            return idx
        elif idx%10 == 4:
            idx = 4*(idx_list[1]-1)+idx_list[0]-1+48
            return idx

    #先手後手で記号を分ける。p1とp2の和を使って全体の盤面を表示する。
    #先手が丸になるようにする。自分の石が1。
    def __str__(self):
        p1 = self.pieces.copy()
        p2 = self.enemy_pieces.copy()
        MARKS = MARKS1 if self.is_first_player() else MARKS2
        p = [x - y for x, y in zip(p1, p2)]
        text = '''
        z = 4
        -----+-----+-----+-----
         114 | 214 | 314 | 414
        -----+-----+-----+-----
         124 | 224 | 324 | 424
        -----+-----+-----+-----
         134 | 234 | 334 | 434
        -----+-----+-----+-----
         144 | 244 | 344 | 444
        -----+-----+-----+-----
        z = 3
        -----+-----+-----+-----
         113 | 213 | 313 | 413
        -----+-----+-----+-----
         123 | 223 | 323 | 423
        -----+-----+-----+-----
         133 | 233 | 333 | 433
        -----+-----+-----+-----
         143 | 243 | 343 | 443
        -----+-----+-----+-----
        z = 2
        -----+-----+-----+-----
         112 | 212 | 312 | 412
        -----+-----+-----+-----
         122 | 222 | 322 | 422
        -----+-----+-----+-----
         132 | 232 | 332 | 432
        -----+-----+-----+-----
         142 | 242 | 342 | 442
        -----+-----+-----+-----
        z = 1
        -----+-----+-----+-----
         111 | 211 | 311 | 411
        -----+-----+-----+-----
         121 | 221 | 321 | 421
        -----+-----+-----+-----
         131 | 231 | 331 | 431
        -----+-----+-----+-----
         141 | 241 | 341 | 441
        -----+-----+-----+-----
        '''
        for idx, i in enumerate(p):
            if i != 0:
                text = text.replace(str(self.inverse_convert(idx)), MARKS[i])#textに書かれてるものを書き換える
        return text

# ランダムで行動選択
def random_action(state):
    legal_actions = state.legal_actions()
    return legal_actions[random.randint(0, len(legal_actions)-1)]

# アルファベータ法で状態価値計算
def alpha_beta(state, alpha, beta):
    # 負けは状態価値-1
    if state.is_lose():
        return -1

    # 引き分けは状態価値0
    if state.is_draw():
        return  0

    # 合法手の状態価値の計算
    for action in state.legal_actions():
        score = -alpha_beta(state.next(action), -beta, -alpha)
        if score > alpha:
            alpha = score

        # 現ノードのベストスコアが親ノードを超えたら探索終了
        if alpha >= beta:
            return alpha

    # 合法手の状態価値の最大値を返す
    return alpha

# アルファベータ法で行動選択
def alpha_beta_action(state):
    # 合法手の状態価値の計算
    best_action = 0
    alpha = -float('inf')
    for action in state.legal_actions():
        score = -alpha_beta(state.next(action), -float('inf'), -alpha)
        if score > alpha:
            best_action = action
            alpha = score

    # 合法手の状態価値の最大値を持つ行動を返す
    return best_action

# プレイアウト
def playout(state):
    # 負けは状態価値-1
    if state.is_lose():
        return -1

    # 引き分けは状態価値0
    if state.is_draw():
        return  0

    # 次の状態の状態価値
    return -playout(state.next(random_action(state)))

# 最大値のインデックスを返す
def argmax(collection):
    return collection.index(max(collection))

# モンテカルロ木探索の行動選択
def mcts_action(state):
    # モンテカルロ木探索のノード
    class node:
        # 初期化
        def __init__(self, state):
            self.state = state # 状態
            self.w = 0 # 累計価値
            self.n = 0 # 試行回数
            self.child_nodes = None  # 子ノード群

        # 評価
        def evaluate(self):
            # ゲーム終了時
            if self.state.is_done():
                # 勝敗結果で価値を取得
                value = -1 if self.state.is_lose() else 0 # 負けは-1、引き分けは0

                # 累計価値と試行回数の更新
                self.w += value
                self.n += 1
                return value

            # 子ノードが存在しない時
            if not self.child_nodes:
                # プレイアウトで価値を取得
                value = playout(self.state)

                # 累計価値と試行回数の更新
                self.w += value
                self.n += 1

                # 子ノードの展開
                if self.n == 10:
                    self.expand()
                return value

            # 子ノードが存在する時
            else:
                # UCB1が最大の子ノードの評価で価値を取得
                value = -self.next_child_node().evaluate()

                # 累計価値と試行回数の更新
                self.w += value
                self.n += 1
                return value

        # 子ノードの展開
        def expand(self):
            legal_actions = self.state.legal_actions()
            self.child_nodes = []
            for action in legal_actions:
                self.child_nodes.append(node(self.state.next(action)))

        # UCB1が最大の子ノードを取得
        def next_child_node(self):
             # 試行回数nが0の子ノードを返す
            for child_node in self.child_nodes:
                if child_node.n == 0:
                    return child_node

            # UCB1の計算
            t = 0
            for c in self.child_nodes:
                t += c.n
            ucb1_values = []
            for child_node in self.child_nodes:
                ucb1_values.append(-child_node.w/child_node.n+2*(2*math.log(t)/child_node.n)**0.5)

            # UCB1が最大の子ノードを返す
            return self.child_nodes[argmax(ucb1_values)]

    # ルートノードの生成
    root_node = node(state)
    root_node.expand()

    # ルートノードを100回評価
    for _ in range(100):
        root_node.evaluate()

    # 試行回数の最大値を持つ行動を返す
    legal_actions = state.legal_actions()
    n_list = []
    for c in root_node.child_nodes:
        n_list.append(c.n)
    return legal_actions[argmax(n_list)]

# 動作確認
if __name__ == '__main__':
    # 状態の生成
    state = State()

    # ゲーム終了までのループ
    while True:
        # ゲーム終了時
        if state.is_done():
            break

        print(state.legal_actions(), len(state.legal_actions()))

        # 次の状態の取得
        state = state.next(random_action(state))


        # 文字列表示
        print(state)
        print(sum(state.pieces) + sum(state.enemy_pieces))
        print()