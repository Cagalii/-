import random
import math
import scipy.stats as ss

# t分布の臨界値計算
def calc_tvt(vt, alpha = 0.01):
    tvt = ss.t.ppf(1 - alpha/2, vt)
    return tvt

# 平均の計算
def calc_mean(loss_rates):
    return sum(loss_rates) / len(loss_rates)

# 不偏分散の計算
def calc_std(loss_rates):
    mean = calc_mean(loss_rates)
    var = sum((x - mean) ** 2 for x in loss_rates) / (len(loss_rates) - 1)
    std_dev = math.sqrt(var)
    return std_dev

# t検定
def t_test(m_s1, m_s2, std_s1, std_s2, n_sim, m_sim):
    test = (m_s1 - m_s2) / math.sqrt((std_s1**2/n_sim)+(std_s2**2/m_sim))
    return test

# 自由度ν
def calc_v(std_s1,std_s2,n_sim,m_sim):
    v = ((std_s1**2/n_sim)+(std_s2**2/m_sim))**2/((std_s1**4)/(n_sim**2*(n_sim-1))+(std_s2**4)/(m_sim**2*(m_sim-1)))
    return v

# 指数分布に従う乱数
def rand_exp(lam):
    u = random.random() #(0, 1)の乱数を生成
    rnd = -1/lam * math.log(1-u)
    return rnd

# シミュレーション関数
def mmss(lam, mu, S, Tend):
    ta = 0        #客の到着時刻
    td = [0] * S  # 各サーバの利用終了時刻
    n_c = 0       #到着した客数
    n_l = 0       #ロスした客数

    while ta < Tend:
          # 2.1客の発生
          ta = ta + rand_exp(lam) # taを更新
          n_c += 1 #　到着した客数を増やす
          # 2.2 客の受付処理
          for s in range(S):
              if td[s] < ta:
                  td[s] = ta + rand_exp(mu)
                  break
          else:
              n_l += 1

    return n_l/n_c

# パラメータの設定
lam_list = [1.0, 2.0, 5.0]  # 平均到着率のリスト
lam = 2.0
mu = 0.1      # 客のサーバ利用時間が平均 1/mu
S = 5        # サーバの台数
Tend_list = list(range(100, 1001, 100))  # シミュレーション終了時刻のリスト
n_sim = 40  # 各終了時間に対するシミュレーションの回数
m_sim = 20  # シミュレーションの回数

for lam in lam_list:
    # シミュレーションとt検定
    # 戦略1: 処理速度を2倍にする
    mu_s1 = mu * 2
    loss_rates_s1 = [mmss(lam, mu_s1, S, Tend_list[-1]) for _ in range(n_sim)]

    # 戦略2: サーバ数を2倍にする
    S_s2 = S * 2
    loss_rates_s2 = [mmss(lam, mu, S_s2, Tend_list[-1]) for _ in range(m_sim)]

    # 平均と標準誤差の計算
    m_s1 = calc_mean(loss_rates_s1)
    m_s2 = calc_mean(loss_rates_s2)
    std_s1 = calc_std(loss_rates_s1)
    std_s2 = calc_std(loss_rates_s2)

    # t検定
    t = t_test(m_s1, m_s2, std_s1, std_s2, n_sim, m_sim)

    # 自由度
    v = calc_v(std_s1, std_s2, n_sim, m_sim)

    # tv
    tv = calc_tvt(v, 0.01)

    # 結果の表示
    print(f"λ = {lam}")
    print(f"戦略1の平均ロス率: {m_s1:.4f}, 不偏分散: {std_s1:.4f}")
    print(f"戦略2の平均ロス率: {m_s2:.4f}, 不偏分散: {std_s2:.4f}")
    print(f"検定統計量: {t:.4f}")
    print(f"自由度: {v: .4f}")
    print(f"tv(0.005): {tv:.4f}")
    print()

    print("検証結果")
    if abs(t) > tv:
        if m_s1 < m_s2:
            print("戦略1が有効です。")
        else:
            print("戦略2が有効です。")
    else:
        print("有意差がありません。")
    print()
