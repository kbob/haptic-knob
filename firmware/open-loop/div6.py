from math import log2

divisor = 3
dividend_max = 5 * 1024
dividend_max = 65535

def test_m(m):
    s0 = int(log2(m / (1/divisor)))
    # print(f'm={m} s0={s0}')
    s1 = s0 + 1
    for dividend in range(dividend_max):
        xq = dividend // divisor
        aq = dividend * m >> s0
        if xq != aq:
            return False
    # print(f'OK: m={m} s={s0}')
    return True

good = []
for m in range(1, 2**32 // (dividend_max - 1)):
    if test_m(m):
        good.append(m)

def div6(n):
    t0 = (n << 2) + n
    t1 = (t0 << 8) + (t0 << 4) + t0
    t2 = (t1 << 1) + n
    return t2 >> 14

print(div6(1000))

best = min(good, key=lambda n: len(bin(n)))
print(f'best = {best} = {best:#b}')
print(f'bests = {[n for n in good if len(bin(n)) == len(bin(best))]}')
