# About
- 各レイヤの API を使ったの publish/subscribe 例。
- **allocator を利用する等リアルタイム向けの実装になっていないのでそのままでは RT ベンチマークに使えません。**
- 関連記事
  - https://c3.isp.co.jp/knowledge/open.knowledge/view/2162 ROS 2 で低レイヤな API を叩いて publish する
  - https://c3.isp.co.jp/knowledge/open.knowledge/view/2167 ROS 2 で低レイヤな API を叩いて subscribe する


# Build
- ROS 2 は 2020/02 時点のもの、つまり E -> F 中の trunk を利用
- ROS 2 ビルド後、環境変数をセットアップ.

```
# ROS 2 master をビルドした環境にパスを通す
. ros2_ws/install-master/local_setup.sh

git clone https://c3.isp.co.jp/gitbucket/git/y-okumura/ros2_hello_worlds.git
cd ros2_hello_worlds/rclcpp
colcon build
```

- 以下のエラーが出た場合、https://c3.isp.co.jp/knowledge/open.knowledge/view/2158 に従い対応する

```
--- stderr: message                                                                                         
CMake Error at /ros2_ws/install/fastrtps/share/fastrtps/cmake/fastrtps-config.cmake:50 (find_package):
  By not providing "Findfoonathan_memory.cmake" in CMAKE_MODULE_PATH this
  project has asked CMake to find a package configuration file provided by
  "foonathan_memory", but CMake did not find one.

  Could not find a package configuration file provided by "foonathan_memory"
  with any of the following names:

    foonathan_memoryConfig.cmake
    foonathan_memory-config.cmake

  Add the installation prefix of "foonathan_memory" to CMAKE_PREFIX_PATH or
  set "foonathan_memory_DIR" to a directory containing one of the above
  files.  If "foonathan_memory" provides a separate development package or
  SDK, be sure it has been installed.
```

# Run
- talker 用と listener 用にそれぞれ別ターミナルを用意するのが吉。
  talker は C-c するまで publish し、 listener は 1 秒 wait を 10 回繰り返す。
  listener は subscribe すると受信したメッセージを標準出力します。

```
. build/local_setup.sh

# rclcpp
./build/cpp_pubsub/talker
./build/cpp_pubsub/listener

# rcl
./build/rcl_pubsub/talker_rcl
./build/rcl_pubsub/listener_rcl

# rmw
./build/rmw_pubsub/talker_rmw
./build/rmw_pubsub/listener_rmw
```

# License
- 応相談

# Author
- y-okumura <y-okumura@isp.co.jp>
