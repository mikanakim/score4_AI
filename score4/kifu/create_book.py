from collections import defaultdict
import csv
import os
import copy
import game

symmetry = [[3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12, 19, 23, 27, 31, 18, 22, 26, 30, 17, 21, 25, 29, 16, 20, 24, 28, 35, 39, 43, 47, 34, 38, 42, 46, 33, 37, 41, 45, 32, 36, 40, 44, 51, 55, 59, 63, 50, 54, 58, 62, 49, 53, 57, 61, 48, 52, 56, 60], 
            [15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48], 
            [12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3, 28, 24, 20, 16, 29, 25, 21, 17, 30, 26, 22, 18, 31, 27, 23, 19, 44, 40, 36, 32, 45, 41, 37, 33, 46, 42, 38, 34, 47, 43, 39, 35, 60, 56, 52, 48, 61, 57, 53, 49, 62, 58, 54, 50, 63, 59, 55, 51], 
            [3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12, 19, 18, 17, 16, 23, 22, 21, 20, 27, 26, 25, 24, 31, 30, 29, 28, 35, 34, 33, 32, 39, 38, 37, 36, 43, 42, 41, 40, 47, 46, 45, 44, 51, 50, 49, 48, 55, 54, 53, 52, 59, 58, 57, 56, 63, 62, 61, 60], 
            [15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0, 31, 27, 23, 19, 30, 26, 22, 18, 29, 25, 21, 17, 28, 24, 20, 16, 47, 43, 39, 35, 46, 42, 38, 34, 45, 41, 37, 33, 44, 40, 36, 32, 63, 59, 55, 51, 62, 58, 54, 50, 61, 57, 53, 49, 60, 56, 52, 48], 
            [12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3, 28, 29, 30, 31, 24, 25, 26, 27, 20, 21, 22, 23, 16, 17, 18, 19, 44, 45, 46, 47, 40, 41, 42, 43, 36, 37, 38, 39, 32, 33, 34, 35, 60, 61, 62, 63, 56, 57, 58, 59, 52, 53, 54, 55, 48, 49, 50, 51], 
            [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31, 32, 36, 40, 44, 33, 37, 41, 45, 34, 38, 42, 46, 35, 39, 43, 47, 48, 52, 56, 60, 49, 53, 57, 61, 50, 54, 58, 62, 51, 55, 59, 63]]

load_file_path = './kifu/kifu_for_book.csv'
save_file_path = './kifu/opening_book.csv'

with open(load_file_path) as f:
    reader = csv.reader(f)
    game_records = [[int(cell) for cell in row] for row in reader]  # 全要素をintに変換

# 定石生成のパラメータ
max_moves = 40  # 定石を生成する最大手数
min_games_per_sequence = 5  # 定石として認めるための最低対局数
inf = float('inf')

def calculate_score(win, vacant):
    """
    勝敗と空きマスの数からスコアを計算する。
    :param win: 勝敗 (1: 先手勝利, -1: 後手勝利, 0: 引き分け)
    :param vacant: 空きマスの数
    :return: スコア
    """
    if win == 1:
        return 1 + 0.01 * vacant
    elif win == -1:
        return -1 - 0.01 * vacant
    else:
        return 0

def aggregate_game_records(records):
    """
    棋譜データを集計してスコアを計算する。
    :param records: 棋譜データ（[棋譜..., 勝敗, 空きマスの数] のリスト）
    :return: 集計結果（辞書形式）
    """
    aggregated = defaultdict(lambda: [0, 0])  # {棋譜の部分列: [対局数, 総スコア]}
    
    for record in records:
        game_record = record[:-2]  # 棋譜
        result = record[-2]       # 勝敗
        vacant = record[-1]       # 空きマス数
        
        score = calculate_score(result, vacant)
        partial_record = []
        
        for move in game_record:
            partial_record.append(move)
            key = tuple(partial_record)
            aggregated[key][0] += 1  # 対局数を加算
            aggregated[key][1] += score  # スコアを加算

    return aggregated

def generate_opening_book(aggregated_records, max_moves, min_games):
    """
    集計結果をもとに定石を生成する。
    :param aggregated_records: 集計された棋譜データ
    :param max_moves: 定石を生成する最大手数
    :param min_games: 定石として認めるための最低対局数
    :return: 定石（辞書形式）
    """
    opening_book = {}
    
    for record, (num_games, total_score) in aggregated_records.items():
        if len(record) > max_moves or num_games < min_games:
            continue
        
        average_score = total_score / num_games
        best_move = record[-1]
        opening_book[record[:-1]] = (best_move, average_score)
    
    return opening_book

def save_opening_book_to_csv(opening_book, filename):
    """
    定石をCSVファイルに保存
    :param opening_book: 定石データ（辞書形式）
    :param filename: 保存先ファイル名
    """
    with open(filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        # ヘッダーを書き込む
        writer.writerow(['手順', '最善手', 'スコア'])
        
        # 重複チェック用のセット
        existing_keys = set()
        
        # 定石データを書き込む
        for record, (best_move, score) in opening_book.items():
            sym_tmp = []
            sym_record = []
            sym_best_move = []
            sym_best_move.append(best_move)
            sym_record.append(record)

            for i in range(len(symmetry)):
                for j in range(len(record)):
                    sym_tmp.append(symmetry[i][record[j]])
                sym_record.append(sym_tmp)
                sym_best_move.append(symmetry[i][best_move])
                sym_tmp = []

            for i in range(len(sym_record)):
                board = ['0'] * 64
                for j in range(len(sym_record[i])):
                    # 先手は1、後手は-1
                    if j % 2 == 0:
                        board[sym_record[i][j]] = '1'
                    else:
                        board[sym_record[i][j]] = '-1'

                # キーとして盤面状態を文字列化
                key = ','.join(board)

                # 重複チェック
                if key in existing_keys:
                    continue  # 重複している場合はスキップ

                # 重複していなければセットに追加し、CSVに書き込む
                existing_keys.add(key)
                writer.writerow([key, sym_best_move[i], score])


# 定石を生成
aggregated = aggregate_game_records(game_records)
opening_book = generate_opening_book(aggregated, max_moves, min_games_per_sequence)

# 定石の出力例
# for record, (best_move, score) in opening_book.items():
#     print(f"手順: {record}, 最善手: {best_move}, スコア: {score}")

save_opening_book_to_csv(opening_book, save_file_path)

print('New books have been generated!')