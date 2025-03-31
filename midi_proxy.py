import serial  
import mido  
import argparse  

def create_master_volume_sysex(level=100):  
    """ 创建主音量SysEx消息 (0-127)  
    基于：MIDI_CMD_SYSTEM_EXCLUSIVE + DevID + SubID1 + SubID2  
    """  
    return mido.Message('sysex',   
        data=[  
            0x7F,  # 非实时通用系统信息  
            0x7F,  # 设备ID (广播)  
            0x04,  # 子ID1 (设备控制)  
            0x01,  # 子ID2 (主音量)  
            0x00,  # 数据空间地址  
            level & 0x7F  # 确保数值在0-127范围内  
        ]  
    )  

parser = argparse.ArgumentParser(description='发送MIDI文件到串口设备')  
parser.add_argument('port', type=str, help='串口设备路径，例如 /dev/cu.usbserial-69526606B6')  
parser.add_argument('midi_file', type=str, help='MIDI文件路径，例如 /path/to/file.mid')  
parser.add_argument('volume', type=int, help='音量大小，0-127')
args = parser.parse_args()
 
mid = mido.MidiFile(args.midi_file, clip=True, debug=True)  
 
volume_msg = create_master_volume_sysex(args.volume)

port = serial.Serial(args.port, 115200) 
port.write(volume_msg.bin())  # 转换为字节并发送  

# print midi info 
# mid.ticks_per_beat
# mid.length
print("ticks_per_beat: ", mid.ticks_per_beat)
print("length: ", mid.length)

for msg in mid.play():  
    b = msg.bin()  
    print(b)  
    port.write(b) 
