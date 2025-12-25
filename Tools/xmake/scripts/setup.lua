import("net.http")
import("utils.archive")

http.download("https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v4/SDKs-v4.zip", "./tmp/SDKs.zip")
archive.extract("./tmp/SDKs.zip", "./SDKs")
os.rmdir("./tmp")