template<typename T>
class Singleton {
  protected:
  Singleton() = default;

  public:
  Singleton(const Singleton &) = delete;

  Singleton &operator=(const Singleton &) = delete;

  static T &instance()
  {
    static T instance;
    return instance;
  }
};

