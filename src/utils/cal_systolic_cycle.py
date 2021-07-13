def calculate_systolic_cycle(systolic_x, systolic_y, input_x, input_y, weight_x, weight_y, cycle_to_write_back_all_output):
    if input_y != weight_x:
        print("input_y should equals to weight x")
        exit(1)

    total_cycle = 0

    # systolic x in the number of the rows
    # systolic y in the number of the cols

    # input x is the number of the input rows
    # input y and wight x is the number of cols, that is the feature dims

    # weight y is the output dim

    # how many steps to go through all input nodes
    steps = int((input_x+systolic_x-1)/systolic_x)

    # for each round of input nodes, how many steps to generate all output dimms
    output_steps = int((weight_y+systolic_y-1)/systolic_y)

    for st in range(steps-1):
        for est in range(output_steps-1):
            # the calculation cycles in systolic array
            # systolic x: for the last input to reach the systolic array
            # input y, for the last element in last input to reach systolic array
            # systolic y: for the last element to leave the array, so it's done
            total_cycle += systolic_x+systolic_y+input_y-2

            # write back output
            total_cycle += cycle_to_write_back_all_output
            pass
        # handle the remainig not full output dim
        remaining_cols = weight_y-((output_steps-1)*systolic_y)
        assert(remaining_cols <= systolic_y)
        assert(remaining_cols > 0)
        total_cycle += systolic_x+remaining_cols+input_y-2
        total_cycle += cycle_to_write_back_all_output

    # new we have finished all Full rows of nodes, next for the last remaining partial rows, first go throght all full cols
    remaining_rows = input_x-((steps-1)*systolic_x)
    if remaining_rows != 0:
        for _ in range(output_steps-1):
            total_cycle += remaining_rows+systolic_y+input_y-2
            total_cycle += cycle_to_write_back_all_output

        # finally, calulate the final parital input nodes and partial output dim
        remaining_cols = weight_y-((output_steps-1)*systolic_y)

        total_cycle += remaining_rows+remaining_cols+input_y-2
        total_cycle += cycle_to_write_back_all_output
    return total_cycle


# tests

if __name__ == "__main__":
    # test a simple case

    cycle = calculate_systolic_cycle(systolic_x=2, systolic_y=3, input_x=3,
                                     input_y=100, weight_x=100, weight_y=4, cycle_to_write_back_all_output=0)

    # noticed there, there will be one 2x3 full fram, one 2x1 partial fram, one 1x3 partial fram, one 1x1 partial frame, so the totcal cycle should be:105-2+103-2+104-2+102-2, will be 406
    assert(cycle == 406)
