# Android-eeprom-jni
Android平台 eeprom JNI代码

1、相关文件

(1)在应用中需要定义一个类，类名必须为EepromJni ，且包名必须为 com.eeprom.jni

如下：

package com.eeprom.jni;


public class EepromJni {

    static {
        System.loadLibrary("eeprom_jni");
    }

    public static native  int EEPROMInit();
    public static native  int EEPROMRead(char[] bufArr, int len);
    public static native  int EEPROMWrite(char[] bufArr, int len);
    public static native  int EEPROMClose();

}

(2)此类 正常运行依赖于系统中/system/lib/下的libeeprom_jni.so







APP使用：
package com.example.eepromtest;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import com.eeprom.jni.EepromJni;
import android.util.Log;

public class MainActivity extends AppCompatActivity {

    public static String TAG = "MainActivity";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        int len = 0;

        EepromJni.EEPROMInit();

        char[] bw = { 0xfd, 0xaa, 0xbb, 0xcc};
        char[] br = new char[4];

 /* 从eeprom的0x00地址读取4个字节数据 */
        br[0] = 0x00;

        len = EepromJni.EEPROMRead(br, 4);
        Log.d(TAG, "0x00 EEPROMRead byte:" + len);
        for (char c:br
                ) {
            Log.d(TAG, "0x00 read byte:" + ((c & 0xff00) >> 8) + (c & 0xff));
        }

     /* 从eeprom的0xfd地址写入4个字节数据 */
        len = EepromJni.EEPROMWrite(bw, 4);
        Log.d(TAG, "EEPROMWrite 0xfd byte:" + len);


       /* 从eeprom的0x00地址读取4个字节数据 */
        br[0] = 0x00;
        len = EepromJni.EEPROMRead(br, 4);
        Log.d(TAG, "0x00 EEPROMRead byte:" + len);
        for (char c:br
             ) {
            Log.d(TAG, "0x00 read byte:" + ((c & 0xff00) >> 8) +(c & 0xff));
        }

  /* 从eeprom的0xfd地址读取4个字节数据 */
        br[0] = 0xfd;
        len = EepromJni.EEPROMRead(br, 4);
        Log.d(TAG, "0xfd EEPROMRead byte:" + len);
        for (char c:br
                ) {
            Log.d(TAG, "0xfd read byte:"+ ((c & 0xff00) >> 8) +(c & 0xff));
        }

        EepromJni.EEPROMClose();


    }
}

说明:以上就向eeprom 中0xfd地址 写入了3个byte数据 分别为0xaa, 0xbb, 0xcc。
API说明
 public static native  int EEPROMInit()

方法功能：初始化eeprom的相关信息，必须在读写取数据前调用
方法参数：无
方法返回值：   0  表示初始化成功
< 0  表示初始化失败

public static native  int EEPROMRead(char[] bufArr, int len)

方法功能：向指定地址读取eeprom数据
方法参数：len 表示buffArr的长度（要读取的地址 + 要读取的数据长度）
          bufArr 表示存放要读取的地址和从eeprom读出来的数据的数据 

注意：bufArr[0]应该存放要读取的eeprom地址（范围 0x00 ~ 0xff）
bufArr[1]  ~  bufArr[len - 1] 存放读取出来的数据

当EEPROM读取地址 + 读取数据长度 > 256(EEPROM_SIZE) - 1时
读取的实际数据长度是256(EEPROM_SIZE) - EEPROM读取地址 

方法返回值：  > 0  表示读取数据的个数
< 0  表示读取失败










public static native  int EEPROMWrite(char[] bufArr, int len)

方法功能：向指定地址写eeprom数据
方法参数：len 表示buffArr的长度（要写入的地址 + 要写入的数据长度）
          bufArr 表示存放要写入的地址和写入eeprom的数据

注意：bufArr[0]应该存放要写入的eeprom地址（范围 0x00 ~ 0xff）
bufArr[1]  ~  bufArr[len - 1] 存放写入的数据

当EEPROM写入地址 + 写入数据长度 > 256(EEPROM_SIZE) - 1时
写入的实际数据长度是256(EEPROM_SIZE) - EEPROM写入地址 

一次性只能写入16Byte个数据长度，根据硬件上的eeprom决定，
超出的字节部分将从写入的地址开始覆盖

方法返回值：  > 0  表示写入数据的个数
< 0  表示写入失败

public static native  int EEPROMClose();

方法功能：销毁eeprom的相关信息，必须在读写取数据完后调用
方法参数：无
方法返回值：   0  表示初始化成功
< 0  表示初始化失败

