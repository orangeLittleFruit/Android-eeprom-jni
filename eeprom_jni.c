#include <string.h>
#include <jni.h>
#include <string.h>
#include <jni.h>
#include <fcntl.h> 
#include <android/log.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <stdio.h>



#define  LOG_TAG    "EEPROM_JNI"
//#define  NDEBUG 0
#include <cutils/log.h>

#define  I2C_DEVICE_NAME  "/dev/i2c-1"  //device point
#define  EEPROM_I2C_SLAVE_ADDR  0x50   // 7bit addr
#define  EEPROM_SIZE  256
typedef  unsigned char   uchar;

/* This is the structure as used in the I2C_RDWR ioctl call */
struct i2c_rdwr_ioctl_data {
	struct i2c_msg __user *msgs;	/* pointers to i2c_msgs */
	__u32 nmsgs;			/* number of i2c_msgs */
};


static int eeprom_fd = -1;
struct i2c_rdwr_ioctl_data g_eeprom_data;



/*
 *  eeprom_read
 *  len ：要读取数据的长度
 *  buf[0]  要读取数据的地址
 *	buf[0] ~ buf[len - 1] 存放读入的数据
 */
 
static int eeprom_read(int fd, uchar buf[], int len)  
{  
	int ret = 0;

    /*eeprom*/
     g_eeprom_data.nmsgs = 2;//读eeprom需要两条消息

	ALOGV("JNI eeprom_read e2prom addr:0x%x", buf[0]); 
	 
    (g_eeprom_data.msgs[0]).len = 1; //第一条消息实际是写eeprom，需要告诉eeprom需要读数据的地址，因此长度为1个字节
    (g_eeprom_data.msgs[0]).addr = EEPROM_I2C_SLAVE_ADDR; // e2prom 设备地址
    (g_eeprom_data.msgs[0]).flags = !I2C_M_RD;//先是写
    (g_eeprom_data.msgs[0]).buf = &buf[0];//e2prom上需要读的数据的地址
	
    (g_eeprom_data.msgs[1]).len = len - 1; //第二条消息才是读eeprom，
    (g_eeprom_data.msgs[1]).addr = EEPROM_I2C_SLAVE_ADDR;// e2prom 读取数据地址 
    (g_eeprom_data.msgs[1]).flags = I2C_M_RD;//然后是读
    (g_eeprom_data.msgs[1]).buf = &buf[1];//存放返回值的地址。
  
    ret = ioctl(fd ,I2C_RDWR ,(unsigned long)&g_eeprom_data);//通过ioctl进行实际的读操作

	if(ret < 0)
    {
		ALOGI("JNI eeprom_read ioctl is error: ret:%d!", ret);
		return ret;
    }
 
	return (len - 1);
}
 

/*
 *  eeprom_write
 *  len ：要写入数据的长度 + 1(要写入的地址)
 *  buf[0]  要写入数据的地址
 *	buf[1] ~ buf[len - 1] 要写入的数据
 *
 */
 
static int eeprom_write(int fd, const uchar buf[], int len)  
{  
	int ret = 0;
    /*写eeprom*/
	
	ALOGV("JNI eeprom_write e2prom addr:0x%x", buf[0]); 
	
	
	if(((int)buf[0] +  (len - 1)) > (EEPROM_SIZE - 1)) /* 要写的数据长度 + 地址超过 eeprom 可寻址范围（即大小）*/
		len = EEPROM_SIZE - (int)buf[0] + 1; /* 能写的个数 */
	
     g_eeprom_data.nmsgs = 1;//由前面eeprom读写分析可知，写eeprom需要一条消息
    
	(g_eeprom_data.msgs[0]).len = len; //此消息的长度为len个字节，第一个字节是要写入数据的地址，后面的字节是要写入的数据
    (g_eeprom_data.msgs[0]).addr = EEPROM_I2C_SLAVE_ADDR;//e2prom 设备地址
    (g_eeprom_data.msgs[0]).flags = !I2C_M_RD; //写
    (g_eeprom_data.msgs[0]).buf = (uchar*)buf;
 
    ret = ioctl(fd ,I2C_RDWR ,(unsigned long)&g_eeprom_data);//通过ioctl进行实际写入操作，后面会详细分析 
	if(ret < 0)
    {
		ALOGI("JNI eeprom_write ioctl is error ###################### ret:%d!", ret);
		return ret;
    }

	return (len - 1);
	
}  


/*
 * JNI函数的命名规则 Java_包名_类名_方法名字
 *  Java_com_eeprom_jni_EepromJni_EEPROMInit 中com_eeprom_jni是包名  EepromJni是类名  EEPROMInit是方法名
 *  编译后生产的库必须以lib开头，如libeeprom_jni.so
 */

JNIEXPORT jint 
Java_com_eeprom_jni_EepromJni_EEPROMInit(JNIEnv* env)
{
	int ret = 0;
	
	ALOGD("----EEPROMInit-----eeprom class init");

	eeprom_fd = open(I2C_DEVICE_NAME, O_RDWR);//打开设备

	ALOGV("eepromClass_Init()-> eeprom_fd = %d  ",eeprom_fd);

	if(eeprom_fd  < 0){
	   ALOGE("open device %s error ", I2C_DEVICE_NAME);
	   return -1;
	}

	g_eeprom_data.nmsgs = 2;//读eeprom需要两条消息
	//ALOGI("g_eeprom_data.nmsgs:%d", g_eeprom_data.nmsgs);
	g_eeprom_data.msgs = (struct i2c_msg*)calloc(g_eeprom_data.nmsgs , sizeof(struct i2c_msg));//分配空间
	if(!g_eeprom_data.msgs)
	{
		ALOGI("EEPROMInit calloc g_eeprom_data is failed!!!");
		return -1;
	}


	usleep(3000);

	ioctl(eeprom_fd ,I2C_TIMEOUT ,3);/*超时时间*/
	ioctl(eeprom_fd ,I2C_RETRIES ,3);/*重复次数*/

   
   return 0;
   
}

JNIEXPORT jint
Java_com_eeprom_jni_EepromJni_EEPROMRead(JNIEnv *env, jobject obj, jcharArray jcharArr, jint len)
{
	jchar *jcharTempBuffer = NULL;  
	uchar *ucharBuffer = NULL;

	int ret = 0,  i = 0;  

	if (len <= 0) {  
		ALOGE("EEEPROMRead: buf len <=0");  
		goto err0;  
	}  

	jcharTempBuffer = (*env)->GetCharArrayElements(env, jcharArr, NULL);
	if(NULL == jcharTempBuffer){
		ALOGE("EEPROMRead: GetCharArrayElements from jcharArr failed!!!");
		goto err0;
	}

	ucharBuffer = (uchar *)calloc(len, sizeof(uchar));
	if(NULL == ucharBuffer){
		ALOGE("EEPROMRead : calloc memory for ucharBuffer is failed!!!");
		goto err1;
	}
	  	  	 
	ucharBuffer[0] = (uchar)jcharTempBuffer[0]; /* 指定读数据地址  由用户指定 */
	 
	ret = eeprom_read(eeprom_fd, ucharBuffer, len);
	if(ret < 0){
		ALOGI("JNI EEPROMRead is  failed!");
		goto err2;
	}

	for (i = 0; i < len; i++)  
		jcharTempBuffer[i] = (jchar)ucharBuffer[i];  /* 拷贝读取的数据到用户buf */

	//ALOGV("EEPROMRead [0]:0x%x [1]:0x%x [2]:0x%x [3]:0x%x  ...... in i2c read jni ", ucharBuffer[0], ucharBuffer[1], ucharBuffer[2], ucharBuffer[3]);  

	(*env)->ReleaseCharArrayElements(env, jcharArr, jcharTempBuffer, 0);//释放资源
	
	free(ucharBuffer);
	ucharBuffer = NULL;
	jcharTempBuffer = NULL;

	return ret;  /* 返回读取数据的个数 */
	  
err2:
	free(ucharBuffer);
	ucharBuffer = NULL;
err1:  
	(*env)->ReleaseCharArrayElements(env, jcharArr, jcharTempBuffer, 0);//释放资源
	jcharTempBuffer = NULL;
err0:  
    return -1; 
}

JNIEXPORT jint
Java_com_eeprom_jni_EepromJni_EEPROMWrite(JNIEnv *env, jobject obj, jcharArray jcharArr, jint len)
{
	jchar *jcharTempBuffer = NULL;  
	uchar *ucharBuffer = NULL; 

	int ret = 0,  i = 0, j = 0;  
	 
	if (len <= 0) {  
	  ALOGE("EEPROMWrite I2C: buf len <=0!!!");  
	  goto err0;  
	}  

	jcharTempBuffer = (*env)->GetCharArrayElements(env, jcharArr, NULL);
	if(NULL == jcharTempBuffer){
		ALOGE("EEPROMWrite: GetCharArrayElements from jcharArr failed!!!");
		goto err0;
	}

	ucharBuffer = (uchar *)calloc(len, sizeof(uchar));
	if(NULL == ucharBuffer){
		ALOGE("EEPROMWrite: calloc memory for ucharBuffer is failed!!!");
		goto err1;
	}

	for (i = 0; i < len; i++)  
		ucharBuffer[i] = (uchar)jcharTempBuffer[i];   /* 拷贝用户buf */


	ret = eeprom_write(eeprom_fd, ucharBuffer, len);
		if(ret < 0){
		ALOGI("JNI EEPROMWrite is  failed!!!");
		goto err2;
	}

	//ALOGV("EEPROMWrite  [0]:0x%x [1]:0x%x  [2]:0x%x [3]:0x%x   ...... in i2c write jni ", ucharBuffer[0], ucharBuffer[1], ucharBuffer[2], ucharBuffer[3]);  
	(*env)->ReleaseCharArrayElements(env, jcharArr, jcharTempBuffer, 0);//释放资源  

	free(ucharBuffer);
	ucharBuffer = NULL;	
	jcharTempBuffer = NULL;

	return ret;
	  
err2:
	free(ucharBuffer);
	ucharBuffer = NULL;
err1:  
	(*env)->ReleaseCharArrayElements(env, jcharArr, jcharTempBuffer, 0);//释放资源 
	jcharTempBuffer = NULL;
err0:  
    return -1; 
}

JNIEXPORT void 
Java_com_eeprom_jni_EepromJni_EEPROMClose(JNIEnv* env)
{
	int ret = 0;

	eeprom_fd = -1;
	free(g_eeprom_data.msgs);
	g_eeprom_data.msgs = NULL;
	close(eeprom_fd);

}
