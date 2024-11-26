import copy
import game
import csv
from pack_for_kifu import *

state = game.State()
replay_state = game.State()
file_path = './kifu.csv'

# csvファイル
with open(file_path) as f:
    kifu = list(csv.reader(f))
    kk = copy.deepcopy(kifu)

# ファイル内をint化
for i in range(len(kifu)):
    # リストの要素を数値化（最終要素を除く）
    kifu[i] = [int(kifu[i][v]) for v in range(len(kifu[i]) - 1)]
    kifu[i].append(kk[i][-1])    
    # リストの最後の要素をチェックして整数化
    if kifu[i] and isinstance(kifu[i][-1], str) and kifu[i][-1].isdigit():
        kifu[i][-1] = int(kifu[i][-1])

new = []
while True:
    print('  ***  Home  ***')
    print('「数字」: 棋譜を入力')
    print('「list」: リストを入力')
    print('「unmove」: 1手戻る')
    print('「a」: 全棋譜を参照')
    print('「b」: 現在の棋譜を参照')
    print('「w」: 手番と現況を確かめる')
    print('「save」: 保存して終了')
    letter1 = input()
    print()
    
    # 数字を入力したとき
    if letter1.isdigit():
        letter1 = int(letter1)
        if state.convert(letter1) not in new:
            if 'w' in new or 'd' in new or 'l' in new:
                print('これ以上置けません(∵ w/d/l)', end = '\n\n')
                continue
            else:
                new.append(int(state.convert(letter1)))
                print('[', end = '')
                for i, element in enumerate(new):
                    if i != len(new) - 1:
                        print(state.inverse_convert(element), ', ', end = '')
                    else:
                        print(state.inverse_convert(element), end = '')
                print(']', end = '\n')
                #勝敗判定の組み込み
                for id in new:
                    state = state.next(id)
                if state.is_lose():
                    if len(new) % 2 == 1:
                        print('先手の勝ち')
                        new.append('w')
                    else:
                        print('後手の勝ち')
                        new.append('l')
                elif state.is_draw():
                    print('引き分け')
                    new.append('d')
                
                else:
                    if len(new) % 2 == 0:
                        print('次、先手: ', len(new)+1, '手目')
                    else:
                        print('次、後手: ', len(new)+1, '手目')
                    print(end = '\n\n')
                state = game.State()
        else:
            print('重複しています', end = '\n\n')
    
    # 「list」
    elif letter1 == 'list':
        tmp = []
        print('リストを入力してください')
        input_str = input()
        print()
        new = list(map(int, input_str.split()))
        new_state = game.State()

        if new[-1] == 1000:#先手の勝ち
            new[-1] = 'w'
        elif new[-1] == 2000:#先手の負け
            new[-1] = 'l'
        elif new[-1] == 3000:#引き分け
            new[-1] = 'd'
        else:
            for i, idx in enumerate(new):
                new_state = new_state.next(idx)
                if new_state.is_lose():
                    if len(new) % 2 == 1:
                        print('先手の勝ち')
                        new.append('w')
                        break
                    else:
                        print('後手の勝ち')
                        new.append('l')
                        break
                elif new_state.is_draw():
                    print('引き分け')
                    new.append('d')
                    break
            new_state = game.State()

        if 'w' in new or 'd' in new or 'l' in new:
            tmp = copy.deepcopy(new)
            del tmp[-1]
        else:
            tmp = copy.deepcopy(new)
        print('[', end = '')
        for i, element in enumerate(tmp):
            if i != len(tmp) - 1:
                print(state.inverse_convert(element), ', ', end = '')
            else:
                print(state.inverse_convert(element), end = '')
        print(']', end = '\n')
        if 'w' in new or 'd' in new or 'l' in new:
            print('対局終了')
        else:
            if len(tmp) % 2 == 0:
                print('次、先手: ', len(new)+1, '手目')
            else:
                print('次、後手: ', len(new)+1, '手目')
            print()
        continue
    
    #全棋譜を参照
    elif letter1 == 'a':
        while True:
            print('「tree」: 棋譜の樹形図を見る')
            print('「r」: 全棋譜を表示')
            print('「数字」: 番号の棋譜を選択')
            print('「n m」: 2つの棋譜を比較(間は半角スペース)')
            print('「back」: 戻る')
            letter2 = input()
            print()
            #樹形図
            if letter2 == 'tree':
                print('win, lose, drawは先手からみた勝敗を表しています')
                print('s, gは直前の分岐後、最初に石を置く方の手番を示しています(先手・後手)')
                y = q_notation(copy.deepcopy(kifu), copy.deepcopy(kifu), [])
                print_tree(y, max_depth(y), kifu)
                print(end = '\n\n')
            #全棋譜表示
            elif letter2 == 'r':
                for i, element in enumerate(kifu):
                    print(i, ': ', end = '')
                    if 'w' in element or 'd' in element or 'l' in element:
                        tmp = copy.deepcopy(element)
                        del tmp[-1]
                    else:
                        tmp = copy.deepcopy(element)
                    print('[', end = '')
                    for j, idx in enumerate(tmp):
                        if j != len(tmp) - 1:
                            print(state.inverse_convert(idx), ', ', end = '')
                        else:
                            print(state.inverse_convert(idx), end = '')
                    print(']', end = '\n\n')
            #棋譜を選択
            elif letter2.isdigit():
                letter2 = int(letter2)
                lst = kifu[letter2]
                while True:
                    print('「d」: 盤面表示')
                    print('「r」: replay')
                    print('「cpp」: c++用出力')
                    print('「wdl」: 勝敗を付与')
                    print('「cor_wdl」: 勝敗を訂正')
                    print('「delete」: この棋譜を消去')
                    print('「back」: 戻る')
                    letter3 = input()
                    print()
                    esc_loop = False

                    #盤面表示
                    if letter3 == 'd':
                        display_state(state, lst)
                        state = game.State()

                    # 盤面再生 replay
                    elif letter3 == 'r':
                        replay(replay_state, lst)
                        replay_state = game.State()

                    #cpp用変換
                    elif letter3 == 'cpp':
                        cpp_convert(state, lst)

                    #勝敗を付与
                    elif letter3 == 'wdl':
                        give_wdl(lst)

                    #勝敗を訂正
                    elif letter3 == 'cor_wdl':
                        correct_wdl(lst)

                    #棋譜削除
                    elif letter3 == 'delete':
                        while True:
                            print('「y/n」: 本当に削除しますか？')
                            letter7 = input()
                            print()
                            if letter7 == 'y':
                                del kifu[letter2]
                                esc_loop = True
                                break
                            else:
                                break    

                    #戻る
                    elif letter3 == 'back':
                        break

                    if esc_loop == True:
                        break
            #２つの棋譜を比較
            elif is_two_numbers(letter2):
                n, m = map(int, letter2.split())
                lst = [copy.deepcopy(kifu[n]), copy.deepcopy(kifu[m])]
                common_elements, idx = extract_common_elements(lst[0], lst[1])
                lst.append(copy.deepcopy(common_elements))# lst = [lst1, lst2, common_lst]
                
                if 'w' in lst[2] or 'd' in lst[2] or 'l' in lst[2]:
                    tmp = copy.deepcopy(lst[2])
                    del tmp[-1]
                else:
                    tmp = copy.deepcopy(lst[2])

                print('盤面表示')
                
                if 'w' in lst[2] or 'd' in lst[2] or 'l' in lst[2]:
                    for i in range(len(lst[2])-1):
                        state = state.next(lst[2][i])
                else:
                    for i in lst[2]:
                        state = state.next(i)
                print(state, end = '\n')
                state = game.State()
                #先手か後手か
                if idx % 2 == 1:
                    print('次、先手: ', len(lst[2])+1, '手目')
                else:
                    print('次、後手: ', len(lst[2])+1, '手目')
                print()
                print(n, ', ', m, 'の共通部分')
                cpp_convert(state, lst[2])
                for i in range(2):
                    lst[i] = lst[i][idx+1:]
                    if 'w' in lst[i] or 'd' in lst[i] or 'l' in lst[i]:
                        tmp = copy.deepcopy(lst[i])
                        del tmp[-1]
                    else:
                        tmp = copy.deepcopy(lst[i])
                    if i == 0:
                        print(n, ' 続き')
                    else:
                        print(m, ' 続き')
                    print('[', end = '')
                    for i, element in enumerate(tmp):
                        if i != len(tmp) - 1:
                            print(state.inverse_convert(element), ', ', end = '')
                        else:
                            print(state.inverse_convert(element), end = '')
                    print(']', end = '\n\n')
                
            #戻る
            elif letter2 == 'back':
                break
                
    #現在の棋譜を参照
    elif letter1 == 'b':
        while True:
            print('「d」: 盤面表示')
            print('「r」: 1手ずつreplay')
            print('「cpp」: c++用出力')
            print('「wdl」: 勝敗を付与')
            print('「cor_wdl」: 勝敗を訂正')
            print('「back」: 戻る')
            letter6 = input()
            print()
            #盤面表示
            if letter6 == 'd':
                display_state(state, new)
                state = game.State()

            # 盤面再生 replay
            elif letter6 == 'r':
                replay(replay_state, new)
                replay_state = game.State()

            #cpp出力
            elif letter6 == 'cpp':
                cpp_convert(state, new)

            #勝敗を付与
            elif letter6 == 'wdl':
                give_wdl(new)

            #勝敗を訂正
            elif letter6 == 'cor_wdl':
                correct_wdl(new)

            #戻る
            elif letter6 == 'back':
                break

    #入力した数字の削除
    elif letter1 == 'unmove':
        if len(new) == 0:
            print('現在、棋譜はありません', end = '\n\n')
        else:
            if 'w' in new or 'd' in new or 'l' in new:
                del new[-1]
            del new[-1]
            print('[', end = '')
            for i, element in enumerate(new):
                if i != len(new) - 1:
                    print(state.inverse_convert(element), ', ', end = '')
                else:
                    print(state.inverse_convert(element), end = '')
            print(']', end = '\n')
            if len(new) % 2 == 0:
                print('次、先手', end = '\n\n')
            else:
                print('次、後手', end = '\n\n')
    
    #手番と現況を確かめる
    elif letter1 == 'w':
        if 'w' in new or 'd' in new or 'l' in new:
            print(len(new)-1, '手まで')
            if new[-1] == 'w':
                print('先手の勝ちです')
            elif new[-1] == 'd':
                print('引き分けです')
            elif new[-1] == 'l':
                print('後手の勝ちです')
        else:
            if len(new) % 2 == 1:
                print('次、後手: ', len(new)+1, '手目')
            else:
                print('次、先手: ', len(new)+1, '手目')

        if 'w' in new or 'd' in new or 'l' in new:
            tmp = copy.deepcopy(new)
            del tmp[-1]
        else:
            tmp = copy.deepcopy(new)
        print('[', end = '')
        for i, element in enumerate(tmp):
            if i != len(tmp) - 1:
                print(state.inverse_convert(element), ', ', end = '')
            else:
                print(state.inverse_convert(element), end = '')
        print(']', end = '\n\n')
            
    #保存して終了          
    elif letter1 == 'save':
        if len(new) > 0:
            new_list = copy.deepcopy(new)
            if 'w' in new or 'd' in new or 'l' in new:
                del new_list[-1]
            check_kifu = []
            for i, sublist in enumerate(copy.deepcopy(kifu)):
                if 'w' in sublist or 'd' in sublist or 'l' in sublist:
                    del sublist[-1]
                    check_kifu.append(sublist)
                else:
                    check_kifu.append(sublist)
            # 重複してなければ追加
            if new_list not in check_kifu:
                kifu.append(new)
                kifu = sorted(kifu, key=len)
        
        # with open(file_path, mode='wb') as fo:
        #     pickle.dump(kifu, fo)
        with open(file_path, 'w') as f:
            writer = csv.writer(f)
            writer.writerows(kifu)
        break