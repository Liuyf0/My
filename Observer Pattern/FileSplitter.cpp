class FileSplitter
{
	string m_filePath;
	int m_fileNumber;

public:
	FileSplitter(const string& filePath, int fileNumber) :
		m_filePath(filePath),
		m_fileNumber(fileNumber) {

	}

	void split() {

		// 1.��ȡ���ļ�

		// 2.��������С�ļ���д��
		for (int i = 0; i < m_fileNumber; ++i) {
			//...

		}
	}
};