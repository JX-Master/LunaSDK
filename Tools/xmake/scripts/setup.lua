import("net.http")
import("utils.archive")

http.download("https://github.com/JX-Master/LunaSDK-ThirdPartySDKs/releases/download/v3/SDKs-v3.zip", "./tmp/SDKs.zip")
archive.extract("./tmp/SDKs.zip", "./SDKs")
os.rmdir("./tmp")