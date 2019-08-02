//--------------------------------------������˵����-------------------------------------------
//* ��������������Kinect��ppt��ʾϵͳ
//* ������������IDE�汾��Visual Studio 2013
//* ������������OpenCV�汾��	3.0 beta
//* ����������ʹ��Ӳ����	KinectV2 Xbox
//* ����ϵͳ��Windows 10
//* Kinect SDK�汾��KinectSDK-v2.0-PublicPreview1409-Setup 
//* 2017��4�� Created by @������  hu_nobuone@163.com
//------------------------------------------------------------------------------------------------

//--------------------------------------ͷ�ļ��������ռ�-------------------------------------------
#include "mykinect.h"
#include <iostream>
//#include <sphelper.h>//����ͷ�ļ�
//#include <sapi.h>
#include<Windows.h>
#include<time.h>	//ʱ��ͷ�ļ�
#include<opencv2/opencv.hpp>
#include<math.h>	//������ѧ��ʽ
using namespace std;
using namespace cv;

//--------------------------------------ȫ�ֱ�������-------------------------------------------
bool leftDetection = FALSE;
bool rightDetection = FALSE;
bool beginDetection = FALSE;
bool vDetection = FALSE;				//�ٶȴﵽ��ֵ ��־λ
bool IsDetection = FALSE;			//�ɹ���������־λ
bool HeightDetection = FALSE;		//�߶ȴﵽ��ֵ��־λ
int detecttime;							//������ļ�ʱ
long  framenumber;			//����֡���
long  depthnumber;			//���֡���
static long  irnumber = 0;				//IR֡���
static long  colornumber = 0;			//RGB֡���
float  SpineHeightin, SpineHeightout;	//Spine�ĸ߶�
float  SpineV;							//Spine���ٶ�
static DWORD tout;						//ʱ�����

/// Initializes the default Kinect sensor
HRESULT CBodyBasics::InitializeDefaultSensor()
{
	//�����ж�ÿ�ζ�ȡ�����ĳɹ����
	HRESULT hr;

	//����kinect 
	hr = GetDefaultKinectSensor(&m_pKinectSensor);  //��ȡKinect������
	if (FAILED(hr))
	{
		return hr;
	}

	//�ҵ�kinect�豸
	if (m_pKinectSensor)
	{
		// Initialize the Kinect and get coordinate mapper and the body reader
		IBodyFrameSource* pBodyFrameSource = NULL;//��ȡ�Ǽ�
		IDepthFrameSource* pDepthFrameSource = NULL;//��ȡ�����Ϣ
		IColorFrameSource* pColorFrameSource = NULL;//��ȡ��ɫ��Ϣ
		IBodyIndexFrameSource* pBodyIndexFrameSource = NULL;//��ȡ������ֵͼ

	    //��Kinect
		hr = m_pKinectSensor->Open();    //�ж�Kinect�Ƿ�������

		//coordinatemapper
		if (SUCCEEDED(hr))         //�����������
		{
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);    //�ж�����ӳ���Ƿ�����
		}

		//bodyframe    ÿ��ͼ����ͨ��source��reader��frame������
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);  //��ȡ�Ǽ�������Ϣ  
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);      //��ȡ�Ǽ�������Ϣ
		}
		//color frame
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_ColorFrameSource(&pColorFrameSource); //��ȡ��ɫ������Ϣ
		}

		if (SUCCEEDED(hr))
		{
			hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);    //�򿪲�ɫ������Ϣ
		}

		//depth frame
		if (SUCCEEDED(hr))
		{
			hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);   // ��ȡ�����Ϣ
		}

		if (SUCCEEDED(hr))
		{
			hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);        //��ȡ�����Ϣ
		}

		//body index frame
		if (SUCCEEDED(hr))                                                    
		{
			hr = m_pKinectSensor->get_BodyIndexFrameSource(&pBodyIndexFrameSource); //��ö�ֵ������ͼ
		}

		if (SUCCEEDED(hr))
		{
			hr = pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader);     //��ȡ��ֵ������ͼ
		}

		SafeRelease(pBodyFrameSource);                               //�ͷ�ָ��COM�ڵ�ָ��
		SafeRelease(pDepthFrameSource);
		SafeRelease(pColorFrameSource);
		SafeRelease(pBodyIndexFrameSource);
	}

	if (!m_pKinectSensor || FAILED(hr))                         //kinect ��ʼ��ʧ��
	{
		std::cout << "Kinect initialization failed!" << std::endl;
		return E_FAIL;
	}

	//skeletonImg,���ڻ��Ǽܡ�������ֵͼ��MAT
	skeletonImg.create(cDepthHeight, cDepthWidth, CV_8UC3);
	skeletonImg.setTo(0);

	//depthImg,���ڻ������Ϣ��MAT
	depthImg.create(cDepthHeight, cDepthWidth, CV_8UC1);
	depthImg.setTo(0);

	//colorImg,���ڻ���ɫ��Ϣ��MAT
	colorImg.create(cColorHeight, cColorWidth, CV_8UC3);
	colorImg.setTo(0);

	return hr;
}


/// Main processing function
void CBodyBasics::Update()
{
	//ÿ�������skeletonImg
	skeletonImg.setTo(0);

	//�����ʧ��kinect���򲻼�������
	if (!m_pBodyFrameReader)
	{
		return;
	}

	IBodyFrame* pBodyFrame = NULL;//�Ǽ���Ϣ
	IDepthFrame* pDepthFrame = NULL;//�����Ϣ
	IColorFrame* pColorFrame = NULL;
	IBodyIndexFrame* pBodyIndexFrame = NULL;//������ֵͼ

	//��¼ÿ�β����ĳɹ����
	HRESULT hr = S_OK;

	//---------------------------------------��ȡ������ֵͼ����ʾ---------------------------------
	//if (SUCCEEDED(hr)){
	//	hr = m_pBodyIndexFrameReader->AcquireLatestFrame(&pBodyIndexFrame);//��ñ�����ֵͼ��Ϣ
	//}
	//if (SUCCEEDED(hr)){
	//	BYTE *bodyIndexArray = new BYTE[cDepthHeight * cDepthWidth];//������ֵͼ��8Ϊuchar�������Ǻ�ɫ��û���ǰ�ɫ
	//	pBodyIndexFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, bodyIndexArray);

	//	//�ѱ�����ֵͼ����MAT��
	//uchar* skeletonData = (uchar*)skeletonImg.data;
	//	for (int j = 0; j < cDepthHeight * cDepthWidth; ++j){
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//		*skeletonData = bodyIndexArray[j]; ++skeletonData;
	//	}
	//	delete[] bodyIndexArray;
	//}
	//SafeRelease(pBodyIndexFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����

	////-----------------------��ȡ������ݲ���ʾ--------------------------
	//if (SUCCEEDED(hr)){
	//	hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);//����������
	//}
	//if (SUCCEEDED(hr)){
	//	UINT16 *depthArray = new UINT16[cDepthHeight * cDepthWidth];//���������16λunsigned int
	//	pDepthFrame->CopyFrameDataToArray(cDepthHeight * cDepthWidth, depthArray);

	//	//��������ݻ���MAT��
	//	uchar* depthData = (uchar*)depthImg.data;
	//	for (int j = 0; j < cDepthHeight * cDepthWidth; ++j){
	//		*depthData = depthArray[j];
	//		++depthData;
	//	}
	//	delete[] depthArray;
	//}
	//SafeRelease(pDepthFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����
	//imshow("depthImg", depthImg);
	//cv::waitKey(5);


	//-----------------------��ȡ��ɫ���ݲ���ʾ--------------------------

	UINT nBufferSize_coloar = 0;
	RGBQUAD *pBuffer_color = NULL;
	
	if (SUCCEEDED(hr)){
		hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);//��ò�ɫ����
	}
	if (SUCCEEDED(hr)){
		ColorImageFormat imageFormat = ColorImageFormat_None;
		RGBQUAD* m_pColorRGBX = new RGBQUAD[cColorWidth * cColorHeight];
		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}
		if (SUCCEEDED(hr))
		{
			hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
		}
		if (SUCCEEDED(hr))
		{
			if (imageFormat == ColorImageFormat_Bgra)//����������format����֪�����庬�壬���һ��Ԥ�ȷ����ڴ棬һ����Ҫ�Լ����ռ��  
			{
				hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize_coloar, reinterpret_cast<BYTE**>(&pBuffer_color));
			}
			else if (m_pColorRGBX)
			{
				pBuffer_color = m_pColorRGBX;
				nBufferSize_coloar = cColorWidth * cColorHeight * sizeof(RGBQUAD);
				hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize_coloar, reinterpret_cast<BYTE*>(pBuffer_color), ColorImageFormat_Bgra);
			}
			else
			{
				hr = E_FAIL;
			}
			uchar* p_mat = colorImg.data;
			
			const RGBQUAD* pBufferEnd = pBuffer_color + (cColorWidth * cColorHeight);

			while (pBuffer_color < pBufferEnd)
			{
				*p_mat = pBuffer_color->rgbBlue;
				p_mat++;
				*p_mat = pBuffer_color->rgbGreen;
				p_mat++;
				*p_mat = pBuffer_color->rgbRed;
				p_mat++;

				++pBuffer_color;
			}
			
			//colorImg = ConvertMat(pBuffer_color, cColorWidth, cColorHeight);
		}
		//BYTE *colorArray = new BYTE[cColorHeight * cColorWidth];//��ɫ������8λBYTE
		//pColorFrame->CopyRawFrameDataToArray(cColorHeight * cColorWidth, colorArray);

		////�Ѳ�ɫ���ݻ���MAT��
		//uchar* colorData = (uchar*)colorImg.data;
		//for (int j = 0; j < cColorHeight * cColorWidth; ++j){
		//	*colorData = colorArray[j];
		//	++colorData;
		//	
		//}
		//cout << *colorData << "\t";
		//delete[] colorArray;

		delete[] m_pColorRGBX;

	}
	SafeRelease(pColorFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����
	namedWindow("colorImg", 0);
	resizeWindow("colorImg", 1024, 720);
	imshow("colorImg", colorImg);

	cv::waitKey(5);
	//-----------------------------��ȡ�Ǽܲ���ʾ----------------------------
	if (SUCCEEDED(hr)){
		hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);//��ȡ�Ǽ���Ϣ
	}
	if (SUCCEEDED(hr))
	{
		IBody* ppBodies[BODY_COUNT] = { 0 };//ÿһ��IBody����׷��һ���ˣ��ܹ�����׷��������

		if (SUCCEEDED(hr))
		{
			//��kinect׷�ٵ����˵���Ϣ���ֱ�浽ÿһ��IBody��
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		}

		if (SUCCEEDED(hr))
		{
			//��ÿһ��IBody�������ҵ����ĹǼ���Ϣ�����һ�����
			ProcessBody(BODY_COUNT, ppBodies);
			//Joint joints[JointType_Count];//�洢�ؽڵ���
			//DepthSpacePoint *depthSpacePosition = new DepthSpacePoint[_countof(joints)];
			//Detection(depthSpacePosition);
		}

		for (int i = 0; i < _countof(ppBodies); ++i)
		{
			SafeRelease(ppBodies[i]);//�ͷ�����
		}
	}
	SafeRelease(pBodyFrame);//����Ҫ�ͷţ�����֮���޷�����µ�frame����

}

/// Handle new body data
void CBodyBasics::ProcessBody(int nBodyCount, IBody** ppBodies)
{
	//��¼��������Ƿ�ɹ�
	HRESULT hr;

	//����ÿһ��IBody
	for (int i = 0; i < nBodyCount; ++i)
	{
		IBody* pBody = ppBodies[i];
		if (pBody)//��û�и���������pBody�������bTracked��ʲô����
		{
			BOOLEAN bTracked = false;
			hr = pBody->get_IsTracked(&bTracked);

			if (SUCCEEDED(hr) && bTracked)
			{
				Joint joints[JointType_Count];//�洢�ؽڵ���
				HandState leftHandState = HandState_Unknown;//����״̬
				HandState rightHandState = HandState_Unknown;//����״̬

				//��ȡ������״̬
				pBody->get_HandLeftState(&leftHandState);
				pBody->get_HandRightState(&rightHandState);

				//�洢�������ϵ�еĹؽڵ�λ��
				DepthSpacePoint *depthSpacePosition = new DepthSpacePoint[_countof(joints)];
				ColorSpacePoint *colorSpacePosition = new ColorSpacePoint[_countof(joints)];
				//��ùؽڵ���
				hr = pBody->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(hr))
				{
					for (int j = 0; j < _countof(joints); ++j)
					{
						//���ؽڵ���������������ϵ��-1~1��ת���������ϵ��424*512��
						m_pCoordinateMapper->MapCameraPointToDepthSpace(joints[j].Position, &depthSpacePosition[j]);
						m_pCoordinateMapper->MapCameraPointToColorSpace(joints[j].Position, &colorSpacePosition[j]);
					}


					//------------------------hand state left  and  right-------------------------------
					DrawHandState(depthSpacePosition[JointType_HandLeft], leftHandState);
					DrawHandState(depthSpacePosition[JointType_HandRight], rightHandState);

					//---------------------------body-------------------------------
					DrawBone(joints, depthSpacePosition, JointType_Head, JointType_Neck);
					DrawBone(joints, depthSpacePosition, JointType_Neck, JointType_SpineShoulder);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_SpineMid);
					DrawBone(joints, depthSpacePosition, JointType_SpineMid, JointType_SpineBase);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineShoulder, JointType_ShoulderLeft);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipRight);
					DrawBone(joints, depthSpacePosition, JointType_SpineBase, JointType_HipLeft);

					// -----------------------Right Arm ------------------------------------ 
					DrawBone(joints, depthSpacePosition, JointType_ShoulderRight, JointType_ElbowRight);
					DrawBone(joints, depthSpacePosition, JointType_ElbowRight, JointType_WristRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_HandRight);
					DrawBone(joints, depthSpacePosition, JointType_HandRight, JointType_HandTipRight);
					DrawBone(joints, depthSpacePosition, JointType_WristRight, JointType_ThumbRight);

					//----------------------------------- Left Arm--------------------------
					DrawBone(joints, depthSpacePosition, JointType_ShoulderLeft, JointType_ElbowLeft);
					DrawBone(joints, depthSpacePosition, JointType_ElbowLeft, JointType_WristLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_HandLeft);
					DrawBone(joints, depthSpacePosition, JointType_HandLeft, JointType_HandTipLeft);
					DrawBone(joints, depthSpacePosition, JointType_WristLeft, JointType_ThumbLeft);

					// ----------------------------------Right Leg--------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipRight, JointType_KneeRight);
					DrawBone(joints, depthSpacePosition, JointType_KneeRight, JointType_AnkleRight);
					DrawBone(joints, depthSpacePosition, JointType_AnkleRight, JointType_FootRight);

					// -----------------------------------Left Leg---------------------------------
					DrawBone(joints, depthSpacePosition, JointType_HipLeft, JointType_KneeLeft);
					DrawBone(joints, depthSpacePosition, JointType_KneeLeft, JointType_AnkleLeft);
					DrawBone(joints, depthSpacePosition, JointType_AnkleLeft, JointType_FootLeft);

				/****************************PPT��������******************************************/
					//�����������볬��0.5��ʱ�ż�⣬���򲻼��
					
					
					cout << "ͷ�ĸ߶�" << joints[JointType_Head].Position.Y << endl;
					cout << "�ҽŵĸ߶�"<< joints[JointType_FootLeft].Position.Y << endl;
					cout << "��ŵĸ߶�" << joints[JointType_FootRight].Position.Y << endl;
					cout << "�ڼ����Ǽ�" <<i<< endl;
					if (fabs(joints[JointType_SpineMid].Position.Z) > 0.5)
						Detection(joints);
						//PPTControl(joints);
					



					cDrawHandState(colorSpacePosition[JointType_HandLeft], false);
					cDrawHandState(colorSpacePosition[JointType_HandRight], false);
					cDrawHandState(colorSpacePosition[JointType_HandLeft], leftDetection);
					cDrawHandState(colorSpacePosition[JointType_HandRight], rightDetection);
				}
				delete[] depthSpacePosition;
				delete[] colorSpacePosition;
			}
		}
	}
	namedWindow("skeletonImg", 0);
	resizeWindow("skeletonImg", 640, 480);
	cv::imshow("skeletonImg", skeletonImg);
	cv::waitKey(5);
}
///ppt����
void CBodyBasics::PPTControl(Joint joints[])
{
	//����
	if (fabs(joints[JointType_Head].Position.X - joints[JointType_HandRight].Position.X) > 0.45)
	{
		if (!leftDetection&&!rightDetection)
		{
			keybd_event(39, 0, 0, 0);
			rightDetection = TRUE;

		}

	}
	else
		rightDetection = false;
	//����
	if (fabs(joints[JointType_Head].Position.X - joints[JointType_HandLeft].Position.X) > 0.45)
	{
		if (!leftDetection&&!rightDetection)
		{
			keybd_event(37, 0, 0, 0);
			leftDetection = TRUE;

		}
	}
	else
		leftDetection = false;

	if (fabs(joints[JointType_Head].Position.X - joints[JointType_HandLeft].Position.X) > 0.45)
	{
		if (!leftDetection&&!rightDetection)
		{
			keybd_event(37, 0, 0, 0);
			leftDetection = TRUE;

		}
	}
	else
		leftDetection = false;
}

//������
void CBodyBasics::Detection(Joint joints[])
{
	static double tin, tout;
	//double tframe;

	//����ÿ����10֡�ĸ߶Ȳ�Ӷ������ٶȣ�1,11,12,22
	//���30֡ÿ�룬��ô10֡����0.33�룬
	
	if (framenumber % 11 == 1)		//framenumber��֡���кţ��Լ������
	{
		tin = static_cast<double>(GetTickCount());
		//cout << "tin��" << tin << endl;
		SpineHeightin = joints[JointType_SpineMid].Position.Y;
		cout << "��ǰ֡��Ϊ��" << framenumber << endl;
		//cout << "��ǰSpineHeightin�ĸ߶�Ϊ" << SpineHeightin << "  m"<<endl;
	}
	if (!(framenumber % 11))
	{
		cout << "��ǰ֡��Ϊ��" << framenumber << endl;
		tout = static_cast<double>(GetTickCount());
		//cout << frmamenumber << endl;
		//cout <<"tout��"<< tout << endl;
		//cout << "ÿ10֡����һ���½����ٶ�" << endl;
		SpineHeightout = joints[JointType_SpineMid].Position.Y;
		//cout << "��ǰ֡��Ϊ��" << frmamenumber << endl;
		//  cout << "***********************************" << endl;
		//  cout << "��ǰSpineHeightin�ĸ߶�Ϊ" << SpineHeightin << "  m" << endl;
		//tframe = (tout - tin) / getTickFrequency();
		// cout <<tframe << endl;
		//  cout << getTickFrequency()<<endl;
		//cout << "��ǰSpineHeightout�ĸ߶�Ϊ" << SpineHeightout << "  m" << endl;
		//SpineV = (SpineHeightin - SpineHeightout) / tframe;
		SpineV = SpineHeightin - SpineHeightout;
		//cout << "SpineV�Ƕ��٣���" << SpineV << endl;
		if ((SpineV) > 0.35)	//�����и�����������1.35m/s���������Ҫ����ʵ��������е���	
		{
			vDetection = true;
			stringstream stream0;
			string str, str1;
			stream0 << SpineV;
			stream0 >> str;
			str1 = "�����������µ��ٶ��ǣ� " + str + " m/s\r\n";
			//CString cstr = str1.c_str();
			//pDlg0->m_outedit.ReplaceSel(cstr);
			cout << "�����������µ��ٶ��ǣ�   " << (SpineV) << "    m/s" << endl;

		}
		else vDetection = false;
	}
	//���߶��������Ե���ļ�⣬�����ľ��롣����ת��Ϊspine��foot֮��ĸ߶ȡ�
	if ((joints[JointType_SpineBase].Position.Y - joints[JointType_FootRight].Position.Y) <= 0.28)
	{

		cv::waitKey(15);
		if ((joints[JointType_SpineBase].Position.Y - joints[JointType_FootRight].Position.Y) <= 0.28)
		{
			if (vDetection)
			{

				HeightDetection = TRUE;
				//IsDetection = TRUE;
				vDetection = false;

				stringstream stream0, stream1;
				string str, str1, str2;
				//CString cstr, cstr1, cstr2;
				//cstr1 = "�ɹ�������\r\n";

				stream0 << joints[JointType_SpineBase].Position.Y;
				stream0 >> str;
				str1 = "JointType_SpineBase�ĸ߶��ǣ� " + str + " m\r\n";
				//cstr = str1.c_str();

				stream1 << joints[JointType_FootRight].Position.Y;
				stream1 >> str;
				str2 = "JointType_FootRight�ĸ߶��ǣ� " + str + " m\r\n";
				//cstr2 = str1.c_str();


				cout << "***************" << "�ɹ�������" << "***************" << endl;
				cout << "JointType_SpineBase�ĸ߶���  " << joints[JointType_SpineBase].Position.Y << "\tm" << endl;
				//cout << "***************" << endl;
				cout << "JointType_FootRight�ĸ߶���  " << joints[JointType_FootRight].Position.Y << "\tm" << endl;
				//SaveSkeletonImg();	//��⵽�����¼������浱ǰͼƬ��Ϣ��
				//SaveDepthImg();
				//SpeechDetection();		//����ѯ��

			}
		}
		else
		{
			HeightDetection = FALSE;
		}
	}
}

//��2��������֮��ľ���
double CBodyBasics::Distance(Joint p1, Joint p2)
{
	double dist = 0;
	dist = sqrt(pow(p2.Position.X - p1.Position.X, 2) +
		pow(p2.Position.Y - p1.Position.Y, 2));
	return dist;
}

//���ֵ�״̬
void CBodyBasics::DrawHandState(const DepthSpacePoint depthSpacePosition, HandState handState)
{
	//����ͬ�����Ʒ��䲻ͬ��ɫBGR
	CvScalar color;
	switch (handState){
	case HandState_Open:
		color = cvScalar(255, 0, 0);
		break;
	case HandState_Closed:
		color = cvScalar(0, 255, 0);
		break;
	case HandState_Lasso:
		color = cvScalar(0, 0, 255);
		break;
	default://���û��ȷ�������ƣ��Ͳ�Ҫ��
		return;
	}

	circle(skeletonImg,
		cvPoint(depthSpacePosition.X, depthSpacePosition.Y),
		20, color, -1);


}

void CBodyBasics::cDrawHandState(const ColorSpacePoint colorSpacePosition, bool isHighlighted)
{
	if (isHighlighted)
	circle(colorImg,
		cvPoint(colorSpacePosition.X, colorSpacePosition.Y),
		100, cvScalar(255, 0, 0), -1);
	else
		circle(colorImg,
		cvPoint(colorSpacePosition.X, colorSpacePosition.Y),
		60, cvScalar(255, 0, 0), -1);
}

/// Draws one bone of a body (joint to joint)
void CBodyBasics::DrawBone(const Joint* pJoints, const DepthSpacePoint* depthSpacePosition, JointType joint0, JointType joint1)
{
	TrackingState joint0State = pJoints[joint0].TrackingState;
	TrackingState joint1State = pJoints[joint1].TrackingState;

	// If we can't find either of these joints, exit
	if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
	{
		return;
	}

	// Don't draw if both points are inferred
	if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
	{
		return;
	}

	CvPoint p1 = cvPoint(depthSpacePosition[joint0].X, depthSpacePosition[joint0].Y),
		p2 = cvPoint(depthSpacePosition[joint1].X, depthSpacePosition[joint1].Y);

	// We assume all drawn bones are inferred unless BOTH joints are tracked
	if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
	{
		//�ǳ�ȷ���ĹǼܣ��ð�ɫֱ��
		line(skeletonImg, p1, p2, cvScalar(255, 255, 255), 8);
	}
	else
	{
		//��ȷ���ĹǼܣ��ú�ɫֱ��
		line(skeletonImg, p1, p2, cvScalar(0, 0, 255), 8);
	}
}


/// Constructor
CBodyBasics::CBodyBasics() :
m_pKinectSensor(NULL),
m_pCoordinateMapper(NULL),
m_pBodyFrameReader(NULL){}
//m_pColorFrameReader(NULL)

/// Destructor
CBodyBasics::~CBodyBasics()
{
	SafeRelease(m_pBodyFrameReader);
	SafeRelease(m_pCoordinateMapper);
	//SafeRelease(m_pColorFrameReader);

	if (m_pKinectSensor)
	{
		m_pKinectSensor->Close();
	}
	SafeRelease(m_pKinectSensor);
}



