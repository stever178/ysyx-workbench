final: prev:
{
  mill = (prev.mill.overrideAttrs (oldAttrs: rec {
    version = "0.12.15";

    # 预编译包
    # src = prev.fetchurl {
    #   url = "https://github.com/com-lihaoyi/mill/releases/download/${version}/${version}-assembly";
    #   hash = "sha256-DHslQS/uzwbZVdATQY3pqQgM51W+26x2AckQnDPVoFc=";
    # };

    # mill 从 0.12.10+ 开始，GitHub Release 中不再提供 -assembly 预编译包

    # 源码包
    src = prev.fetchFromGitHub {
      owner = "com-lihaoyi";
      repo = "mill";
      rev = version;  # mill 的 tag 与版本号一致
      hash = "sha256-y1LmZa5TnMdkuOjvmA8nUHLyEv/I4M6xObL+fqxdHmA=";
    };

    buildInputs = oldAttrs.buildInputs ++ [ final.openjdk21 ];
  })).override {
    jre = final.openjdk21;
  };
}
