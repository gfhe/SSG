import struct

def parse(index_file_path, parsed_txt_file_path):
    """
    将ssg 图结构转为text文本（区别在于ssg文件是紧凑型文件，text是可读性文件）
    
    ssg 文件格式和转换成text的格式如下：
    width
    n_eps eps1 eps2 ... epsn_eps
    n1 n1_1 n1_2 ... n1_n1
    n2 n2_1 n2_2 ... n2_n2
    ...
    n_n n_n_1 n_n_2 ... n_n_n_n

    :param index_file_path: ssg 文件路径
    :param parsed_txt_file_path: 转换后的文本文件路径
    """
    with open(parsed_txt_file_path, 'w') as pf:
        with open(index_file_path, 'rb') as file:
            # 读取第一个 unsigned 数据 width
            width = struct.unpack('<I', file.read(4))[0]
            pf.write(f"{width}\n")

            # 读取第二个 unsigned 数据 n_eps
            n_eps = struct.unpack('<I', file.read(4))[0]
            pf.write(f"{n_eps} ")

            # 读取 n_eps 个 unsigned 数据组成的数组
            eps = struct.unpack('<{}I'.format(n_eps), file.read(n_eps * 4))
            pf.write(" ".join(str(item) for item in eps))
            pf.write("\n")

            # 循环迭代读取数据
            while True:
                try:
                    # 读取一个 unsigned 数据 n
                    n = struct.unpack('<I', file.read(4))[0]
                    if n == 0:
                        break

                    # 读取 n1 个 unsigned 数据组成的数组
                    neighbors = struct.unpack('<{}I'.format(n), file.read(n * 4))
                    pf.write(f"{n}")
                    pf.write(" ".join(str(item) for item in neighbors))
                    pf.write("\n")


                except struct.error:
                    break

            pf.flush()
            pf.close()

if __name__ == '__main__': 
    index_file_path = 'data/glove.ssg'
    parsed_txt_file_path = 'data/glove.ssg.txt'
    parse(index_file_path, parsed_txt_file_path)
