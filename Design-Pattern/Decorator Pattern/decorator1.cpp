//ҵ�����
class Stream {
public:
    virtual char Read(int number) = 0;
    virtual void Seek(int position) = 0;
    virtual void Write(char data) = 0;

    virtual ~Stream() {}
};

//������
class FileStream : public Stream {
public:
    virtual char Read(int number) {
        //���ļ���
    }
    virtual void Seek(int position) {
        //��λ�ļ���
    }
    virtual void Write(char data) {
        //д�ļ���
    }

};

class NetworkStream :public Stream {
public:
    virtual char Read(int number) {
        //��������
    }
    virtual void Seek(int position) {
        //��λ������
    }
    virtual void Write(char data) {
        //д������
    }

};

class MemoryStream :public Stream {
public:
    virtual char Read(int number) {
        //���ڴ���
    }
    virtual void Seek(int position) {
        //��λ�ڴ���
    }
    virtual void Write(char data) {
        //д�ڴ���
    }

};