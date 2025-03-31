def file_to_progmem(input_file, output_file, array_name, bytes_per_line=10):  
    with open(input_file, 'rb') as f:  
        data = f.read()  
    
    hex_bytes = [f'0x{byte:02x}' for byte in data]  
    
    with open(output_file, 'w') as f:  
        f.write(f'const unsigned char PROGMEM {array_name}[] = {{\n')  
        
        for i in range(0, len(hex_bytes), bytes_per_line):  
            line = ', '.join(hex_bytes[i:i+bytes_per_line])  
            f.write(f'    {line},\n')  
        
        f.write('};\n')  

if __name__ == "__main__":  
    # 配置参数  
    input_filename = 'input.bin'    # 输入文件路径  
    output_filename = 'data.h'      # 输出头文件名  
    array_name = 'file_data'        # 数组变量名  
    
    file_to_progmem(input_filename, output_filename, array_name)  
    print(f"转换完成！已保存至 {output_filename}")  
