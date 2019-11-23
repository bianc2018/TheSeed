#ifndef  NS_ERROR_HPP_
#define  NS_ERROR_HPP_
namespace ns
{
	namespace util
	{
		namespace error
		{
			enum NS_ERROR_CODE
			{
				//״̬��Ϣ
				//�����Ѿ�����
				NS_TCP_CONNECTED =1,
				//���������ر�
				NS_TCP_CLOSEED   =2,
				//�����յ�һ������
				NS_TCP_RECVED	 =3,
				//���ӷ�����һ������
				NS_TCP_SENDED	 =4,
				//���쳣
				NS_SUCCESS = 0,
				//����������ʧ��
				NS_NEW_BUFF_ERR = -1,
				//tcp�������ݳ�ʱ
				NS_TCP_RECV_TIMEOUT = -2,
				//tcp��������ʧ��
				NS_TCP_RECV_ERROR = -3,
				//tcp�������ݳ�ʱ
				NS_TCP_SEND_TIMEOUT = -4,
				//tcp��������ʧ��
				NS_TCP_SEND_ERROR = -5,
				//tcp �������Ӵ�ʧ��
				NS_TCP_SERVER_OPEN_ERR = -6,
				//tcp ��������������ʧ�� SERVER_SET_ERROR
				NS_TCP_SERVER_SET_ERROR=-7,
				//tcp ������ �󶨵�ַ����
				NS_TCP_SERVER_BIND_ERROR = -8,
				//tcp ������ ���ս�������ʱ���ִ���
				NS_TCP_SERVER_ACCEPT_ERROR = -9,
				//tcp ������  �����ѹر�
				NS_TCP_SERVER_LINK_CLOSED_ERROR = -10,
				//tcp �ͻ���  �����쳣
				NS_TCP_CLIENT_CONNECT_ERROR = -11,
				//tcp �ͻ���  ���ӳ�ʱ
				NS_TCP_CLIENT_CONNECT_TIME_OUT_ERROR = -12,
				//tcp �ͻ���  ���ӹر�
				NS_TCP_CLIENT_CONNECT_CLOSED_ERROR = -13,
				//tcp ����ľ��Ϊ��
				NS_TCP_HANDLER_NULL_ERROR = -14,
				//Ŀ���ļ�������
				NS_FILE_IS_NOT_EXIST = -15,
				//�����RANGE
				NS_BAD_RANGE = -16,
				//�޷����ļ�
				NS_NOT_OPEN_FILE = -17,
				//��Ч���ļ�
				NS_BAD_FILE = -18,
				//��ȡ�ļ�ʧ��
				NS_READ_FILE_ERROR = -19,
				//���ݻ���������Ϊ��
				NS_DATA_CACHE_NULL = -20,
				//���ݻ�������������
				NS_DATA_CACHE_FULL = -21,
				//���ݻ��治�����ȡ������
				NS_DATA_CACHE_CANNOT_GET_EMPYTY = -22,
                //�յ�����
                NS_HTTP_EMPYTY_REQUEST = -23,
                //http ����ʧ��
                NS_HTTP_OBJECT_PARSER_ERROR = -24,
                //���������������
                NS_PARAM_ERROR = -25,
                //����Ϊ��
                NS_HOST_NULL = -26,
				//δ֪����
				NS_UNKNOW_ERROR = -999,

			};
		
            std::string msg(NS_ERROR_CODE e)
            {
                std::string msg = "δ֪����";
                switch (e)
                {
                case ns::util::error::NS_TCP_CONNECTED:
                    msg = "tcp �����Ѿ�����";
                    break;
                case ns::util::error::NS_TCP_CLOSEED:
                    msg = "tcp �����Ѿ��ر�";
                    break;
                case ns::util::error::NS_TCP_RECVED:
                    msg = "tcp ���ӽ��յ�����";
                    break;
                case ns::util::error::NS_TCP_SENDED:
                    msg = "tcp ���ӷ����������";
                    break;
                case ns::util::error::NS_SUCCESS:
                    msg = "�������";
                    break;
                case ns::util::error::NS_NEW_BUFF_ERR:
                    msg = "new ����ʧ��";
                    break;
                case ns::util::error::NS_TCP_RECV_TIMEOUT:
                    msg = "tcp �������ݳ�ʱ";
                    break;
                case ns::util::error::NS_TCP_RECV_ERROR:
                    msg = "tcp ���������쳣";
                    break;
                case ns::util::error::NS_TCP_SEND_TIMEOUT:
                    msg = "tcp �������ݳ�ʱ";
                    break;
                case ns::util::error::NS_TCP_SEND_ERROR:
                    msg = "tcp ���������쳣";
                    break;
                case ns::util::error::NS_TCP_SERVER_OPEN_ERR:
                    msg = "tcp ����������ʧ��";
                    break;
                case ns::util::error::NS_TCP_SERVER_SET_ERROR:
                    msg = "tcp ��������������ʧ��";
                    break;
                case ns::util::error::NS_TCP_SERVER_BIND_ERROR:
                    msg = "tcp ��������ַ��ʧ��";
                    break;
                case ns::util::error::NS_TCP_SERVER_ACCEPT_ERROR:
                    msg = "tcp ��������ַ���տͻ�������ʧ��";
                    break;
                case ns::util::error::NS_TCP_SERVER_LINK_CLOSED_ERROR:
                    msg = "tcp ��������ַ���տͻ������Ӵ��󣬵õ�����������Ч��";
                    break;
                case ns::util::error::NS_TCP_CLIENT_CONNECT_ERROR:
                    msg = "tcp �ͻ�������ʧ��";
                    break;
                case ns::util::error::NS_TCP_CLIENT_CONNECT_TIME_OUT_ERROR:
                    msg = "tcp �ͻ������ӳ�ʱ";
                    break;
                case ns::util::error::NS_TCP_CLIENT_CONNECT_CLOSED_ERROR:
                    msg = "tcp �ͻ��������쳣�ر�";
                    break;
                case ns::util::error::NS_TCP_HANDLER_NULL_ERROR:
                    msg = "�ص�Ϊ��";
                    break;
                case ns::util::error::NS_FILE_IS_NOT_EXIST:
                    msg = "Ŀ���ļ�������";
                    break;
                case ns::util::error::NS_BAD_RANGE:
                    msg = "���������Χ";
                    break;
                case ns::util::error::NS_NOT_OPEN_FILE:
                    msg = "�޷���ָ���ļ�";
                    break;
                case ns::util::error::NS_BAD_FILE:
                    msg = "�ļ��쳣";
                    break;
                case ns::util::error::NS_READ_FILE_ERROR:
                    msg = "�ļ���ȡ�쳣";
                    break;
                case ns::util::error::NS_DATA_CACHE_NULL:
                    msg = "���ݻ������Ϊ��";
                    break;
                case ns::util::error::NS_DATA_CACHE_FULL:
                    msg = "���ݻ����������";
                    break;
                case ns::util::error::NS_DATA_CACHE_CANNOT_GET_EMPYTY:
                    msg = "���ݻ�����в�����ȡsize=0��С������";
                    break;
                case ns::util::error::NS_HTTP_EMPYTY_REQUEST:
                    msg = "�յ�http����";
                    break;
                case ns::util::error::NS_HTTP_OBJECT_PARSER_ERROR:
                    msg = "http ��������";
                    break;
                case ns::util::error::NS_PARAM_ERROR:
                    msg = "��������Ĳ�������";
                    break;
                case ns::util::error::NS_HOST_NULL:
                    msg = "����Ϊ��";
                    break;
                case ns::util::error::NS_UNKNOW_ERROR:
                    break;
                default:
                    break;
                }
            
                return msg;
            }
        }
	}
}
#endif // ! NS_ERROR_HPP_
