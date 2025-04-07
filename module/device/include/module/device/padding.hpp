#define MODULE_DEVICE_PADDING(start, stop, name)                                                                                      \
                                                                                                                                      \
  private:                                                                                                                            \
                                                                                                                                      \
	char name [[maybe_unused]][(stop) - (start)];                                                                                     \
                                                                                                                                      \
  public:
