protocol_fields = {}
lua_f = open("descriptor.lua", "rb")
c_line = 0

for line in lua_f:
    line_values = line.split()
    dict_list = [line_values[0].decode()]
    size = line_values[1].decode()
    if size == 'uint8':
        dict_list.append(2)
    elif size == 'uint16':
        dict_list.append(4)
    else:
        continue
    protocol_fields[c_line] = dict_list
    c_line += 1


for key, value in protocol_fields.items():
    print(key, ' : ', value)
lua_f.close()

print(protocol_fields[0][1])
ret_val = True

