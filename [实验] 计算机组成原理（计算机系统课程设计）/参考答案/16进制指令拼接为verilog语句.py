def process_instructions(input_file, output_file):
    try:
        with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
            for line_number, instruction in enumerate(infile):
                instruction = instruction.strip()
                formatted_instruction = f"assign mem[{line_number}] = 32'h{instruction};\n"
                outfile.write(formatted_instruction)
        return True
    except Exception as e:
        print(f"无法打开 {e}")
        return False

# 改成你的路径
work_dir = r"C:\Desktop\cputest\cputest.srcs\\"
input_file = work_dir + 'inst.txt'
output_file = work_dir + 'inst_result.txt'

process_instructions(input_file, output_file)