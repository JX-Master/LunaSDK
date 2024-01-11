import("net.http")
import("utils.archive")

http.download("https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v2/SDKs.zip", "./tmp/SDKs.zip")
archive.extract("./tmp/SDKs.zip", "./SDKs")
os.rmdir("./tmp")