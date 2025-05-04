from collections import defaultdict
import csv
import copy
import game

symmetry = [[3, 7, 11, 15, 2, 6, 10, 14, 1, 5, 9, 13, 0, 4, 8, 12, 19, 23, 27, 31, 18, 22, 26, 30, 17, 21, 25, 29, 16, 20, 24, 28, 35, 39, 43, 47, 34, 38, 42, 46, 33, 37, 41, 45, 32, 36, 40, 44, 51, 55, 59, 63, 50, 54, 58, 62, 49, 53, 57, 61, 48, 52, 56, 60], 
            [15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48], 
            [12, 8, 4, 0, 13, 9, 5, 1, 14, 10, 6, 2, 15, 11, 7, 3, 28, 24, 20, 16, 29, 25, 21, 17, 30, 26, 22, 18, 31, 27, 23, 19, 44, 40, 36, 32, 45, 41, 37, 33, 46, 42, 38, 34, 47, 43, 39, 35, 60, 56, 52, 48, 61, 57, 53, 49, 62, 58, 54, 50, 63, 59, 55, 51], 
            [3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12, 19, 18, 17, 16, 23, 22, 21, 20, 27, 26, 25, 24, 31, 30, 29, 28, 35, 34, 33, 32, 39, 38, 37, 36, 43, 42, 41, 40, 47, 46, 45, 44, 51, 50, 49, 48, 55, 54, 53, 52, 59, 58, 57, 56, 63, 62, 61, 60], 
            [15, 11, 7, 3, 14, 10, 6, 2, 13, 9, 5, 1, 12, 8, 4, 0, 31, 27, 23, 19, 30, 26, 22, 18, 29, 25, 21, 17, 28, 24, 20, 16, 47, 43, 39, 35, 46, 42, 38, 34, 45, 41, 37, 33, 44, 40, 36, 32, 63, 59, 55, 51, 62, 58, 54, 50, 61, 57, 53, 49, 60, 56, 52, 48], 
            [12, 13, 14, 15, 8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3, 28, 29, 30, 31, 24, 25, 26, 27, 20, 21, 22, 23, 16, 17, 18, 19, 44, 45, 46, 47, 40, 41, 42, 43, 36, 37, 38, 39, 32, 33, 34, 35, 60, 61, 62, 63, 56, 57, 58, 59, 52, 53, 54, 55, 48, 49, 50, 51], 
            [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15, 16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31, 32, 36, 40, 44, 33, 37, 41, 45, 34, 38, 42, 46, 35, 39, 43, 47, 48, 52, 56, 60, 49, 53, 57, 61, 50, 54, 58, 62, 51, 55, 59, 63]]


load_file_path = './kifu/opening_book.csv'
load_human_book_path = './kifu/human_made_book.csv'
save_human_book_path = './kifu/opening_book_with_human_made_book.csv'

import csv

def read_opening_book_from_csv(filename):
    """
    定石データをCSVファイルから読み込む
    :param filename: 読み込むCSVファイル名
    :return: 読み込んだ定石データ（辞書形式）
    """
    opening_book = {}
    
    with open(filename, mode='r') as file:
        reader = csv.reader(file)

        next(reader)

        for row in reader:
            # 盤面のデータをリストに変換
            board = row[0]
            # 最善手とスコアを取得
            best_move = int(row[1])
            score = float(row[2])
            
            # 辞書に保存
            opening_book[board] = (best_move, score)
    
    return opening_book

def read_human_book(human_book_filename):
    with open(human_book_filename) as f:
        reader = csv.reader(f)
        human_book = [[int(cell) for cell in row] for row in reader]  # 全要素をintに変換
    return human_book

def convert_human_book(human_book):
    human_made_book = {}
    for elem in human_book:
        human_made_book[tuple(elem[:-1])] = (elem[-1], 1.0)
    return human_made_book

def add_to_opening_book(opening_book, human_made_book):
    """
    定石辞書に要素を追加する。
    :param opening_book: 定石辞書（盤面をキー、定石リストを値とする）
    :param human_book: 盤面を表す文字列（例: "0,0,0,...,0"）
    :param move: 最善手
    :param score: スコア
    """
    
    # 定石データを書き込む
    for record, (best_move, score) in human_made_book.items():
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
            num_count = [0]*16
            board = ['0'] * 64
            for j in range(len(sym_record[i])):
                # 先手は1、後手は-1
                idx = sym_record[i][j]
                if j % 2 == 0:
                    board[16*num_count[idx] + idx] = '1'
                    num_count[idx] += 1
                else:
                    board[16*num_count[idx] + idx] = '-1'
                    num_count[idx] += 1
                    # board[sym_record[i][j]] = '-1'

            # キーとして盤面状態を文字列化
            key = ','.join(board)

            # human made bookは優先度が一番
            opening_book[key] = (sym_best_move[i], score)

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
        
        # 定石データを書き込む
        for board, (best_move, score) in opening_book.items():
            # キーとして盤面状態を文字列化
            if isinstance(board, tuple):
                key = ','.join(board)
            else:
                key = board
            # key = ','.join(map(str, board))

            # 重複していなければセットに追加し、CSVに書き込む
            writer.writerow([key, best_move, score])


opening_book = read_opening_book_from_csv(load_file_path)
human_book = read_human_book(load_human_book_path)
human_made_book = convert_human_book(human_book)
opening_book = add_to_opening_book(opening_book, human_made_book)
save_opening_book_to_csv(opening_book, save_human_book_path)

print('human_made_book has been added!')