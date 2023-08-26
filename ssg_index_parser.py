import struct


def parse(index_file_path):
    """
    解析 ssg 图结构

    ssg 图结构文件格式如下：
    width
    n_eps eps1 eps2 ... epsn_eps
    n1 n1_1 n1_2 ... n1_n1
    n2 n2_1 n2_2 ... n2_n2
    ...
    n_n n_n_1 n_n_2 ... n_n_n_n

    :param index_file_path: ssg 文件路径
    """

    with open(index_file_path, 'rb') as file:
        try:
            # 读取第一个 unsigned 数据 width
            width = struct.unpack('<I', file.read(4))[0]

            # 读取第二个 unsigned 数据 n_eps
            n_eps = struct.unpack('<I', file.read(4))[0]

            # 读取 n_eps 个 unsigned 数据组成的数组
            eps = struct.unpack('<{}I'.format(n_eps), file.read(n_eps * 4))

            vertex_num = 0
            # 循环迭代读取数据
            while True:

                # 读取一个 unsigned 数据 n
                n = struct.unpack('<I', file.read(4))[0]
                if n == 0:
                    break

                vertex_num += 1
                # 读取 n1 个 unsigned 数据组成的数组
                neighbors = struct.unpack('<{}I'.format(n), file.read(n * 4))
                ## process neighbors

        except struct.error:
            print(f"end of file, n_vectors={vertex_num}")


if __name__ == '__main__':
    index_file_path = 'data/glove.ssg'
    parse(index_file_path)
