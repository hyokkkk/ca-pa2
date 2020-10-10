import sys
it = iter(sys.stdin)
# 첫번째줄 스킵
next(it)
for line in it:
    line = line.rstrip().replace('.L', 'Label')
    print(rf'"{line}\n"')
