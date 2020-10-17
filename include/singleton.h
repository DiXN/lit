template<typename T>
class Singleton {
  protected:
  Singleton() = default;

  public:
  Singleton(const Singleton &) = default;

  Singleton &operator=(const Singleton &) = default;

  static T &instance()
  {
    static T instance;
    return instance;
  }
};

