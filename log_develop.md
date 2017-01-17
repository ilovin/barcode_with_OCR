## 开发日志
### Work Sheet
分辨率：1654*2340
#### 校正部分
要求必须有两个条形码，且要有框,在行数>0.1的位置需要有一个有横线的条形码
#### OCR部分
OCR要求开头必须是字母
对于有两个条形码的
使用高度为70的粗框选，随后进一步框选
对于只有一个条形码的，使用该条码校正图片，并使用高度40的的直接进行OCR
手工去除了不是字母和数字的部分，并截取了两个字符之间的内容作为最终输出
#### 固定阈值部分
imgEnhance: Mat kernal size
drewCompute: blur(size) kernal.size, erode ,dilate
getBarCodePostion: blur.size kernal.size threshold
findOcrRect: 
    1 bar case : ocrRect.height 40
    more bar case: below the barcode 70
#### 条形码编码处理方面与OCR字符串的处理
认为包含SHA的即为serial
并要求OCR部分的字符串长度>5

#### 时间开销
- drew compute 0.5
- getBar 0.4
- findOcr 0.001
- deocde 0.1/each ~ 0.4
- OCR and putText 0.01