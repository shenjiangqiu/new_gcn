from collections import deque, OrderedDict
import numpy as np
from typing import Iterable


class BloomFilter:
    def __init__(self, entries: int):
        self.entries = entries
        self.array = np.zeros((entries,))

        pass

    def hash1(self, node_id: int) -> int:
        return node_id % self.entries

    def hash2(self, node_id: int) -> int:
        return (node_id * 0x12323123) % self.entries

    def insert(self, edges_id: Iterable[int]):
        for i in edges_id:
            self.array[self.hash1(i)] += 1
            self.array[self.hash2(i)] += 1

        pass

    def delete(self, edges_id: Iterable[int]):
        for i in edges_id:
            if self.array[self.hash1(i)] != 0:
                self.array[self.hash1(i)] -= 1
            else:
                print("error , it's zero!!")
            if self.array[self.hash2(i)] != 0:
                self.array[self.hash2(i)] -= 1
            else:
                print("error, it's zero!!")
        pass

    # get the score for a output node
    def query(self, edges_id: Iterable[int]) -> int:
        total_score = 0
        for i in edges_id:
            total_score += (self.array[self.hash1(i)] + self.array[self.hash2(i)])
        return total_score
        pass

    pass


class Window:
    def __init__(self, input_num: int, output_num: int, edges: int, all_edges: Iterable[tuple[int, int]] = ()):
        self.input_num = input_num
        self.output_num = output_num
        self.edges = edges
        self.all_edges = all_edges
        pass


def find_max_in_dict(counting_dict: dict[int, int]) -> int:
    max_v = 0
    max_index = 0
    for i in counting_dict:
        if counting_dict[i] > max_v:
            max_v = counting_dict[i]
            max_index = i
    return max_index


class Graph:
    def __init__(self, name):
        window_list = list[Window]
        self.__sliding_windows = window_list()
        self.__dense_windows = window_list()
        self.__sequential_window = window_list()
        self.__current_short_long_queue_window = window_list()
        self.__outer_short_long_queue_window = window_list()
        self.__global_min_window = window_list()
        self.__bloom_short_long = window_list()
        self.__bloom_and_neighbor = window_list()
        self.__max_output_num = 0
        self.__max_input_num = 0
        self.__setup_window_finished = False
        self.__setup_hardware_finished = False
        self.__array = []
        self.__feature_size = 0
        with open(name) as file:
            line = file.readline().split()
            #
            assert (line[0] == 'f')
            self.feature_size = int(line[1])

            lines = file.readlines()
            if len(lines[-1].strip()) == 0:
                lines = lines[:-1]
            else:
                lines = lines[:]

            for i, line in enumerate(lines):
                all_nodes = [int(i) for i in line.split()]

                all_nodes.append(i)
                self.__array.append(set(all_nodes))

    def print_result(self):
        for i in [self.__bloom_short_long, self.__bloom_and_neighbor, self.__outer_short_long_queue_window,
                  self.__current_short_long_queue_window,
                  self.__sequential_window, self.__dense_windows, self.__sliding_windows]:

            print(f"{len(i)}")

    def print(self) -> None:
        print(f"{self.__feature_size} ")
        for i in self.__array:
            print()
            for j in i:
                print(f"{j}", end=",")

        print()

    def setup_hardware(self, max_input_num: int, max_output_num: int) -> None:
        self.__setup_hardware_finished = True

        self.__max_output_num = max_output_num
        self.__max_input_num = max_input_num

    def setup_window(self, divider: int = 5, max_queue_size: int = 100, dynamic_bloom: bool = False,
                     bloom_entries: int = 4096) -> None:
        if not self.__setup_hardware_finished:
            print("should setup hardware first")
            exit(1)
        self.__setup_window_finished = True
        self.__setup_sliding_window()
        self.__setup_dense_window()
        self.__setup_seq_window()
        self.__setup_current_short_long(divider)
        self.__setup_outer_short_long(divider, max_queue_size)
        self.__setup_bloom_short_long(divider, dynamic_bloom, max_queue_size, bloom_entries)
        self.__setup_bloom_and_neighbor_short_long(divider, dynamic_bloom, max_queue_size, bloom_entries)
        # policy one, sliding window

    def __setup_sliding_window(self) -> None:
        total_cols = int((len(self.__array) + self.__max_output_num - 1) / self.__max_output_num)
        for i in range(total_cols):
            start = i * self.__max_output_num
            end = min((i + 1) * self.__max_output_num, len(self.__array))
            j = 0
            while j < len(self.__array):
                # skipping
                j = self.__skipping(j, start, end)
                if j == len(self.__array):
                    break
                # find a j as start
                input_len = 0
                total_edges = 0
                last_existing = 0
                starting_row = j
                if j != len(self.__array):
                    # build the window
                    while j < len(self.__array) and input_len < self.__max_input_num:
                        for output in range(start, end):
                            if j in self.__array[output]:
                                total_edges += 1
                                last_existing = j
                        j += 1
                        input_len += 1
                    # finished build the window
                # shrinking here
                self.__sliding_windows.append(Window(last_existing - starting_row + 1, end - start, total_edges))

    def __skipping(self, j: int, start: int, end: int) -> int:
        while j < len(self.__array):
            exist = False
            for out in range(start, end):
                if j in self.__array[out]:
                    exist = True
                    break
            if exist:
                break
            j += 1
        return j

    def __setup_dense_window(self) -> None:
        total_cols = int((len(self.__array) + self.__max_output_num - 1) / self.__max_output_num)
        for i in range(total_cols):
            start = i * self.__max_output_num
            end = min((i + 1) * self.__max_output_num, len(self.__array))
            j = 0
            while j < len(self.__array):
                # skipping
                j = self.__skipping(j, start, end)
                if j == len(self.__array):
                    break
                # find a j as start
                input_len = 0
                total_edges = 0
                if j != len(self.__array):
                    # build the window
                    while j < len(self.__array) and input_len < self.__max_input_num:
                        find_in_row = False
                        for output in range(start, end):
                            if j in self.__array[output]:
                                total_edges += 1
                                last_existing = j
                                find_in_row = True

                        j += 1
                        if find_in_row:
                            input_len += 1
                            # finished build the window
                # shrinking here
                self.__dense_windows.append(Window(input_len, end - start, total_edges))

    def __setup_seq_window(self) -> None:
        # deep copy the working set
        working_set = [i.copy() for i in self.__array]
        current_output_set = deque()
        i = 0
        while len(current_output_set) < self.__max_output_num and i < len(working_set):
            current_output_set.append(i)
            i += 1

        # have remaining output in output set
        while len(current_output_set) != 0:
            # first generate window
            total_edges = 0
            current_selected_output = current_output_set[0]
            # skip all empty output
            all_empty = False
            while len(working_set[current_selected_output]) == 0:
                current_output_set.popleft()
                if len(current_output_set) == 0:
                    all_empty = True
                    break
                current_selected_output = current_output_set[0]

            current_input_size = 0
            this_window_size = len(current_output_set)
            all_edges = []
            if not all_empty:
                while current_input_size < self.__max_input_num:
                    selected_input = working_set[current_selected_output].pop()
                    current_input_size += 1
                    total_edges += 1
                    all_edges.append((selected_input, current_selected_output))
                    # delete all edges from all output node
                    for output in current_output_set:
                        if selected_input in working_set[output]:
                            all_edges.append((selected_input, output))
                            total_edges += 1
                            working_set[output].remove(selected_input)
                    # test if current output is empty
                    all_finished = False
                    while len(working_set[current_selected_output]) == 0:
                        current_output_set.popleft()
                        if len(current_output_set) == 0:
                            all_finished = True
                            break
                        current_selected_output = current_output_set[0]
                    if all_finished:
                        break
                    pass
                # then insert new output
                self.__sequential_window.append(Window(current_input_size, this_window_size, total_edges, all_edges))

            while len(current_output_set) < self.__max_output_num and i < len(working_set):
                current_output_set.append(i)
                i += 1

            pass

        pass

    def __setup_current_short_long(self, divider: int = 2) -> None:
        # deep copy the working set
        working_set = [i.copy() for i in self.__array]
        current_output_set_short = deque()
        current_output_set_long = deque()

        i = 0
        while len(current_output_set_short) + len(current_output_set_long) < self.__max_output_num and i < len(
                working_set):
            if len(self.__array[i]) <= divider:
                current_output_set_short.append(i)
            else:
                current_output_set_long.append(i)
            i += 1

        # have remaining output in output set
        while len(current_output_set_short) != 0 or len(current_output_set_long) != 0:
            # first generate window
            selected_queue = current_output_set_long if len(current_output_set_short) == 0 else current_output_set_short

            total_edges = 0
            current_selected_output = selected_queue[0]
            current_input_size = 0
            this_window_size = len(current_output_set_short) + len(current_output_set_long)
            all_edges = []
            # skip all emtpy entry
            all_empty = False
            while len(working_set[current_selected_output]) == 0:
                selected_queue.popleft()
                if len(selected_queue) == 0:
                    if len(current_output_set_long) == 0:
                        all_empty = True
                        break
                    else:
                        selected_queue = current_output_set_long
                current_selected_output = selected_queue[0]
            if not all_empty:
                while current_input_size < self.__max_input_num:
                    selected_input = working_set[current_selected_output].pop()
                    current_input_size += 1
                    total_edges += 1
                    all_edges.append((selected_input, current_selected_output))
                    # delete all edges from all output node
                    for output in current_output_set_short + current_output_set_long:
                        if selected_input in working_set[output]:
                            all_edges.append((selected_input, output))
                            total_edges += 1
                            working_set[output].remove(selected_input)

                    # test if current output is empty
                    all_finished = False
                    while len(working_set[current_selected_output]) == 0:
                        selected_queue.popleft()
                        if len(selected_queue) == 0:
                            if len(current_output_set_long) == 0:
                                if len(current_output_set_long) == 0:
                                    all_finished = True
                                    break
                                else:
                                    selected_queue = current_output_set_long
                            else:
                                selected_queue = current_output_set_long

                        current_selected_output = selected_queue[0]
                    if all_finished:
                        break
                    pass
                # then insert new output
                self.__current_short_long_queue_window.append(
                    Window(current_input_size, this_window_size, total_edges, all_edges))

            while len(current_output_set_short) + len(current_output_set_long) < self.__max_output_num and i < len(
                    working_set):
                if len(self.__array[i]) <= divider:
                    current_output_set_short.append(i)
                else:
                    current_output_set_long.append(i)
                i += 1

            pass

        pass

    def __setup_outer_short_long(self, divider: int = 2, max_queue_size: int = 4096) -> None:
        # deep copy the working set
        working_set = [i.copy() for i in self.__array]
        outer_output_set_short = deque()
        outer_output_set_long = deque()
        i = 0

        # first setup outer queue
        while len(outer_output_set_short) + len(outer_output_set_long) < max_queue_size and i < len(self.__array):
            if len(self.__array[i]) <= divider:
                outer_output_set_short.append(i)
            else:
                outer_output_set_long.append(i)
            i += 1

        current_output_set = deque()
        # second setup working queue
        while len(current_output_set) < self.__max_output_num:
            if len(outer_output_set_short) != 0:
                current_output_set.append(outer_output_set_short.popleft())
            elif len(outer_output_set_long) != 0:
                current_output_set.append(outer_output_set_long.popleft())

        # start to setup window
        # have remaining output in output set
        while len(current_output_set) != 0:
            # first generate window

            total_edges = 0
            current_selected_output = current_output_set[0]
            current_input_size = 0
            this_window_size = len(current_output_set)
            all_edges = []

            # skip all emtpy entry
            all_empty = False
            while len(working_set[current_selected_output]) == 0:
                current_output_set.popleft()
                if len(current_output_set) == 0:
                    all_empty = True
                    break
                current_selected_output = current_output_set[0]
            if not all_empty:
                while current_input_size < self.__max_input_num:
                    selected_input = working_set[current_selected_output].pop()
                    current_input_size += 1
                    total_edges += 1
                    all_edges.append((selected_input, current_selected_output))
                    # delete all edges from all output node
                    for output in current_output_set:
                        if selected_input in working_set[output]:
                            all_edges.append((selected_input, output))
                            total_edges += 1
                            working_set[output].remove(selected_input)

                    # test if current output is empty
                    all_finished = False
                    while len(working_set[current_selected_output]) == 0:
                        current_output_set.popleft()
                        if len(current_output_set) == 0:
                            all_finished = True
                            break

                        current_selected_output = current_output_set[0]
                    if all_finished:
                        break
                    pass
                # then insert new output
                self.__outer_short_long_queue_window.append(
                    Window(current_input_size, this_window_size, total_edges, all_edges))
            # insert the outter
            # first setup outer queue
            while len(outer_output_set_short) + len(outer_output_set_long) < max_queue_size and i < len(self.__array):
                if len(self.__array[i]) <= divider:
                    outer_output_set_short.append(i)
                else:
                    outer_output_set_long.append(i)
                i += 1

            while len(current_output_set) < self.__max_output_num:
                if len(outer_output_set_short) != 0:
                    current_output_set.append(outer_output_set_short.popleft())
                elif len(outer_output_set_long) != 0:
                    current_output_set.append(outer_output_set_long.popleft())
                else:
                    break

        pass

    # 选择一个与当前workingset 邻居最多的节点
    # 这是一个approximate的算法，并不是进去的时候邻居最多，而是加入队列的时候邻居最多
    # 怎么动态设置long_short 区别
    # 1， 静态long_short
    def __setup_bloom_short_long(self, init_divider: int, dynamic: bool, max_queue_size: int,
                                 bloom_entries: int) -> None:
        divider = init_divider
        # deep copy the working set
        working_set = [i.copy() for i in self.__array]
        outer_output_set_short = deque()
        outer_output_set_long = deque()
        i = 0
        bloom_filter = BloomFilter(entries=bloom_entries)
        # first setup outer queue

        # fix bug here, the score is bigger better
        while len(outer_output_set_short) + len(outer_output_set_long) < max_queue_size and i < len(self.__array):
            if dynamic:
                if len(outer_output_set_short) > len(outer_output_set_long):
                    # short is too much,增加short的门槛， divider越大，short越难进
                    divider += 1
                else:
                    divider = 1 if divider == 1 else divider - 1

            score = bloom_filter.query(self.__array[i])
            if score > divider:
                outer_output_set_short.append(i)
            else:
                outer_output_set_long.append(i)
            bloom_filter.insert(self.__array[i])
            i += 1

        current_output_set = deque()
        # second setup working queue
        while len(current_output_set) < self.__max_output_num:
            if len(outer_output_set_short) != 0:
                current_output_set.append(outer_output_set_short.popleft())
            elif len(outer_output_set_long) != 0:
                current_output_set.append(outer_output_set_long.popleft())

        # start to setup window
        # have remaining output in output set
        while len(current_output_set) != 0:
            # first generate window

            total_edges = 0
            current_selected_output = current_output_set[0]
            current_input_size = 0
            this_window_size = len(current_output_set)
            all_edges = []

            # skip all emtpy entry
            all_empty = False
            while len(working_set[current_selected_output]) == 0:
                current_output_set.popleft()
                if len(current_output_set) == 0:
                    all_empty = True
                    break
                current_selected_output = current_output_set[0]
            if not all_empty:
                while current_input_size < self.__max_input_num:
                    selected_input = working_set[current_selected_output].pop()
                    current_input_size += 1
                    total_edges += 1
                    all_edges.append((selected_input, current_selected_output))
                    # delete all edges from all output node
                    for output in current_output_set:
                        if selected_input in working_set[output]:
                            all_edges.append((selected_input, output))
                            bloom_filter.delete([selected_input])
                            total_edges += 1
                            working_set[output].remove(selected_input)

                    # test if current output is empty
                    all_finished = False
                    while len(working_set[current_selected_output]) == 0:
                        current_output_set.popleft()
                        if len(current_output_set) == 0:
                            all_finished = True
                            break

                        current_selected_output = current_output_set[0]
                    if all_finished:
                        break
                    pass
                # then insert new output
                self.__bloom_short_long.append(
                    Window(current_input_size, this_window_size, total_edges, all_edges))
            # insert the outter
            # first setup outer queue

            # fix bug here!!!, score bigger, it's better
            while len(outer_output_set_short) + len(outer_output_set_long) < max_queue_size and i < len(self.__array):
                score = bloom_filter.query(self.__array[i])
                if dynamic:
                    if len(outer_output_set_short) > len(outer_output_set_long):
                        divider += 1
                    else:
                        divider = 1 if divider == 1 else divider - 1

                if score > divider:
                    outer_output_set_short.append(i)
                else:
                    outer_output_set_long.append(i)
                bloom_filter.insert(self.__array[i])
                i += 1

            while len(current_output_set) < self.__max_output_num:
                if len(outer_output_set_short) != 0:
                    current_output_set.append(outer_output_set_short.popleft())
                elif len(outer_output_set_long) != 0:
                    current_output_set.append(outer_output_set_long.popleft())
                else:
                    break

        pass

    def __setup_bloom_and_neighbor_short_long(self, init_divider: int, dynamic: bool, max_queue_size: int,
                                              bloom_entries: int) -> None:
        divider = init_divider
        # deep copy the working set
        working_set = [i.copy() for i in self.__array]
        outer_output_set_short = deque()
        outer_output_set_long = deque()
        i = 0
        bloom_filter = BloomFilter(entries=bloom_entries)
        # first setup outer queue

        # fix bug here, the score is bigger better
        while len(outer_output_set_short) + len(outer_output_set_long) < max_queue_size and i < len(self.__array):
            if dynamic:
                if len(outer_output_set_short) > len(outer_output_set_long):
                    # short is too much,增加short的门槛， divider越大，short越难进
                    divider += 1
                else:
                    divider = 1 if divider == 1 else divider - 1

            score = bloom_filter.query(self.__array[i])
            if score > divider:
                # print(f"pushing {i} to short")
                outer_output_set_short.append(i)
            else:
                # print(f"pushing {i} to long")
                outer_output_set_long.append(i)
            bloom_filter.insert(self.__array[i])
            i += 1

        current_output_set = set[int]()
        # second setup working queue
        while len(current_output_set) < self.__max_output_num:
            if len(outer_output_set_short) != 0:
                current_output_set.add(outer_output_set_short.popleft())
            elif len(outer_output_set_long) != 0:
                current_output_set.add(outer_output_set_long.popleft())

        # start to setup window
        # have remaining output in output set
        next_select_item = -1
        while len(current_output_set) != 0:
            # first generate window
            if next_select_item > 0:
                if next_select_item not in current_output_set:
                    next_select_item = next(iter(current_output_set))
            else:
                next_select_item = next(iter(current_output_set))

            total_edges = 0
            current_input_size = 0
            this_window_size = len(current_output_set)
            all_edges = []

            # skip all emtpy entry
            all_empty = False
            while len(working_set[next_select_item]) == 0:
                current_output_set.remove(next_select_item)
                if len(current_output_set) == 0:
                    all_empty = True
                    break
                next_select_item = next(iter(current_output_set))
            if not all_empty:
                # mean the output id, and the score, score mean how many times this output have been touched
                next_waiting_set = dict[int, int]()
                while current_input_size < self.__max_input_num:
                    selected_input = working_set[next_select_item].pop()
                    current_input_size += 1
                    total_edges += 1
                    all_edges.append((selected_input, next_select_item))
                    # delete all edges from all output node
                    for output in current_output_set:
                        if selected_input in working_set[output]:
                            if output != next_select_item:
                                if output in next_waiting_set:
                                    next_waiting_set[output] += 1
                                else:
                                    next_waiting_set[output] = 1

                            all_edges.append((selected_input, output))
                            bloom_filter.delete([selected_input])
                            total_edges += 1
                            working_set[output].remove(selected_input)

                    # test if current output is empty
                    all_finished = False
                    while len(working_set[next_select_item]) == 0:
                        current_output_set.remove(next_select_item)
                        if len(current_output_set) == 0:
                            all_finished = True
                            break
                        if len(next_waiting_set) != 0:
                            next_select_item = find_max_in_dict(next_waiting_set)
                            # print(f"select next:{next_select_item},score:{next_waiting_set[next_select_item]}")
                            next_waiting_set.pop(next_select_item)
                        else:
                            next_select_item = next(iter(current_output_set))

                    if all_finished:
                        break
                    pass
                # then insert new output
                self.__bloom_and_neighbor.append(
                    Window(current_input_size, this_window_size, total_edges, all_edges))

            # insert the outter
            # first setup outer queue

            # fix bug here!!!, score bigger, it's better
            while len(outer_output_set_short) + len(outer_output_set_long) < max_queue_size and i < len(self.__array):
                score = bloom_filter.query(self.__array[i])
                if dynamic:
                    if len(outer_output_set_short) > len(outer_output_set_long):
                        divider += 1
                    else:
                        divider = 1 if divider == 1 else divider - 1

                if score > divider:
                    outer_output_set_short.append(i)
                else:
                    outer_output_set_long.append(i)
                bloom_filter.insert(self.__array[i])
                i += 1

            while len(current_output_set) < self.__max_output_num:
                if len(outer_output_set_short) != 0:
                    current_output_set.add(outer_output_set_short.popleft())
                elif len(outer_output_set_long) != 0:
                    current_output_set.add(outer_output_set_long.popleft())
                else:
                    break

        pass

    def print_sliding_window(self) -> None:
        for i in self.__sliding_windows:
            print(f"{i.input_num} {i.output_num} {i.edges}")

    def print_dense_window(self) -> None:
        for i in self.__dense_windows:
            print(f"{i.input_num} {i.output_num} {i.edges}")

    def print_seq_window(self) -> None:
        for i in self.__sequential_window:
            print(f"{i.input_num} {i.output_num} {i.edges} {i.all_edges}")

    def print_current_short_long_window(self) -> None:
        for i in self.__current_short_long_queue_window:
            print(f"{i.input_num} {i.output_num} {i.edges} {i.all_edges}")

    def print_outer_short_long_window(self) -> None:
        for i in self.__outer_short_long_queue_window:
            print(f"{i.input_num} {i.output_num} {i.edges} {i.all_edges}")

    def print_bloom(self) -> None:
        for i in self.__bloom_short_long:
            print(f"{i.input_num} {i.output_num} {i.edges} {i.all_edges}")

    def print_bloom_and_neighbor(self) -> None:
        for i in self.__bloom_and_neighbor:
            print(f"{i.input_num} {i.output_num} {i.edges} {i.all_edges}")

    pass


if __name__ == "__main__":
    graph = Graph("test.graph")
    graph.setup_hardware(2, 2)
    graph.setup_window(dynamic_bloom=True)
    graph.print_result()
