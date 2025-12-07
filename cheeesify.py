# 12/6/2025 - 12/7/2025 (6 7)
import re

# read and parse input
with open('victim.c', 'r') as file:
    content = file.read()

pattern = re.compile(r"""
(
    \n                  |
    [ ]+                |   # spaces
    //                  |
    /\*|\*/             |
    "                   |
    '                   |
    \\                  |   # backslash
    \( | \) | \{ | \} | \[ | \] | ; | # random chars i decided
    \#                  |   # for #define
    \w+                 |   # alphanumeric + underscore
    [^\s\w"'/\\#]+          # any other symbols (group consecutive ones)
)
""", re.VERBOSE)

tokens = pattern.split(content)
tokens = [t for t in tokens if t]

# regex done

cheeses = {} # stores {token, cheese string}
cheese_counter = 0

# generate(1) = "cheese", generate(2) = "Cheese", etc.
def generate(n):
    base = "chee"
    n2 = n
    multiplier = 64
    while (1 << (len(base)+2) <= n2):
        base += "e"
        n2 -= multiplier
        multiplier *= 2
    
    base += "se"
    
    out = ""
    for i in range(len(base)):
        if ((n2 >> i) & 1):
            out += base[i].upper()
        else:
            out += base[i]
    
    return out

# gets cheese from token, generates if doesn't exist
def get_cheese(token):
    global cheese_counter
    if token not in cheeses:
        cheeses[token] = generate(cheese_counter)
        # if a cheese token is already in the code, don't generate it
        while (cheeses[token] in tokens):
            cheese_counter += 1
            cheeses[token] = generate(cheese_counter)
        cheese_counter += 1
        # print define statement
        print("#define %s %s" % (cheeses[token], token))
    return cheeses[token]

# code containing all the cheese tokens
out = ""

# loop through tokens
i = 0
while i < len(tokens):
    token = tokens[i]
    
    # count [i...index of s] as a single token
    def goto_next(s):
        global i, token
        while ((i + 1 < len(tokens))):
            i += 1
            token += tokens[i]
            if (tokens[i] == '\\'):
                i += 1
                token += tokens[i]
                continue
            # stop when found next except when backslash
            if tokens[i] == s:
                break
    
    if token[0] == '\n' or token[0] == ' ':
        pass
    elif token == '"':
        goto_next('"')
    elif token == "'":
        goto_next("'")
    elif token == '//' or token == '#':
        goto_next('\n')
    elif token == '/*':
        goto_next('*/')
    
    # don't cheesify these
    if (token[0] == '#' or token == '\n' or token[0] == ' '):
        out += token
    # replace comments
    elif token[0:2] == '//':
        out += '// cheese\n'
    elif token[0:2] == '/*':
        out += '/* cheese */'
    # cheesify everything else
    else:
        out += get_cheese(token)
        # add space if next token isn't newline or space
        if (i+1 < len(tokens) and tokens[i+1] not in ['\n', ' ']):
            out += ' '
    i += 1

print(out)