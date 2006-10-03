#define MAX_HISTORY_LEN 20

class History
{
public:
  History(void);
  ~History(void);
  void AddToHistoryList(const char *str);
  const char *GetHistoryEntry(int entry);

protected:

private:
  char *m_list[MAX_HISTORY_LEN];
};
