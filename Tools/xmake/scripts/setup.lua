import("net.http")
import("utils.archive")

http.download("https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v1/packages.zip", "./tmp/packages.zip")
archive.extract("./tmp/packages.zip", "./SDKs/packages")
os.rmdir("./tmp")