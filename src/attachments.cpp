#include "attachments.hpp"
#include <wincodec.h>
#include <algorithm>
#include <cstdint>
namespace qc {
namespace {
using Microsoft::WRL::ComPtr;
bool fail(std::wstring& error,const wchar_t* message){error=message;return false;}
bool prepare(const std::filesystem::path& file,UINT maxWidth,UINT maxHeight,uint64_t maxBytes,uint64_t maxPixels,PreparedImage& result,std::wstring& error){
 std::error_code ec;auto bytes=std::filesystem::file_size(file,ec);if(ec||!bytes||bytes>maxBytes)return fail(error,L"Image missing or exceeds the file size limit.");
 ComPtr<IWICImagingFactory> factory;if(FAILED(CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&factory))))return fail(error,L"Cannot initialize the Windows image decoder.");
 ComPtr<IWICBitmapDecoder> decoder;ComPtr<IWICBitmapFrameDecode> frame;if(FAILED(factory->CreateDecoderFromFilename(file.c_str(),nullptr,GENERIC_READ,WICDecodeMetadataCacheOnDemand,&decoder))||FAILED(decoder->GetFrame(0,&frame)))return fail(error,L"Unsupported or damaged image. Choose a PNG, JPEG, BMP or GIF image.");
 UINT width=0,height=0;if(FAILED(frame->GetSize(&width,&height))||!width||!height||uint64_t(width)*height>maxPixels)return fail(error,L"Image dimensions exceed the supported limit.");
 double ratio=(std::min)({1.0,double(maxWidth)/width,double(maxHeight)/height});UINT nextWidth=(std::max)(1U,static_cast<UINT>(width*ratio)),nextHeight=(std::max)(1U,static_cast<UINT>(height*ratio));
 ComPtr<IWICBitmapScaler> scaler;if(FAILED(factory->CreateBitmapScaler(&scaler))||FAILED(scaler->Initialize(frame.Get(),nextWidth,nextHeight,WICBitmapInterpolationModeFant)))return fail(error,L"Cannot resize image.");
 ComPtr<IStream> stream;if(FAILED(CreateStreamOnHGlobal(nullptr,TRUE,&stream)))return fail(error,L"Cannot allocate image stream.");
 ComPtr<IWICBitmapEncoder> encoder;ComPtr<IWICBitmapFrameEncode> output;if(FAILED(factory->CreateEncoder(GUID_ContainerFormatPng,nullptr,&encoder))||FAILED(encoder->Initialize(stream.Get(),WICBitmapEncoderNoCache))||FAILED(encoder->CreateNewFrame(&output,nullptr))||FAILED(output->Initialize(nullptr))||FAILED(output->SetSize(nextWidth,nextHeight)))return fail(error,L"Cannot encode PNG image.");
 WICPixelFormatGUID format=GUID_WICPixelFormat32bppBGRA;if(FAILED(output->SetPixelFormat(&format)))return fail(error,L"Unsupported image pixel format.");
 ComPtr<IWICFormatConverter> converter;if(FAILED(factory->CreateFormatConverter(&converter))||FAILED(converter->Initialize(scaler.Get(),format,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom))||FAILED(output->WriteSource(converter.Get(),nullptr))||FAILED(output->Commit())||FAILED(encoder->Commit()))return fail(error,L"Cannot write PNG image.");
 STATSTG stat{};if(FAILED(stream->Stat(&stat,STATFLAG_NONAME))||stat.cbSize.QuadPart>4*1024*1024)return fail(error,L"The normalized PNG exceeds 4 MiB. Choose a smaller image.");
 LARGE_INTEGER start{};stream->Seek(start,STREAM_SEEK_SET,nullptr);result={std::move(stream),nextWidth,nextHeight};return true;
}
bool managedReference(const std::wstring& reference){
 if(!reference.starts_with(L"attachments/")||!reference.ends_with(L".png")||reference.size()>100||reference.find(L"..")!=reference.npos)return false;
 auto name=reference.substr(12);if(name.size()<5)return false;
 for(auto c:name)if(!((c>=L'a'&&c<=L'z')||(c>=L'A'&&c<=L'Z')||(c>=L'0'&&c<=L'9')||c==L'-'||c==L'_'||c==L'.'))return false;
 return true;
}
}
bool importWorkflowImage(const std::filesystem::path& source,const std::filesystem::path& dataDirectory,std::wstring& reference,std::wstring& error){
 error.clear();PreparedImage image;if(!prepare(source,1600,1600,10*1024*1024,24000000,image,error))return false;
 GUID id{};wchar_t name[40]{};if(FAILED(CoCreateGuid(&id))||!StringFromGUID2(id,name,40))return fail(error,L"Cannot generate an attachment name.");
 auto relative=L"attachments/img-"+std::wstring(name+1,36)+L".png";std::error_code ec;std::filesystem::create_directories(dataDirectory/L"attachments",ec);if(ec)return fail(error,L"Cannot create the attachments folder.");
 auto destination=dataDirectory/relative;HANDLE file=CreateFileW(destination.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);if(file==INVALID_HANDLE_VALUE)return fail(error,L"Cannot save the image attachment.");
 char buffer[16384];ULONG count=0;bool ok=true;HRESULT status=S_OK;while((status=image.stream->Read(buffer,sizeof(buffer),&count))>=0&&count){DWORD written=0;if(!WriteFile(file,buffer,count,&written,nullptr)||written!=count){ok=false;break;}}
 ok=ok&&SUCCEEDED(status)&&FlushFileBuffers(file);CloseHandle(file);if(!ok){DeleteFileW(destination.c_str());return fail(error,L"Image attachment could not be saved.");}
 reference=std::move(relative);return true;
}
bool loadWorkflowImage(const std::filesystem::path& dataDirectory,const std::wstring& reference,UINT maximumWidth,PreparedImage& image,std::wstring& error){
 error.clear();if(!managedReference(reference))return fail(error,L"Only locally attached images are displayed.");
 return prepare(dataDirectory/reference,(std::max)(maximumWidth,1U),1600,4*1024*1024,2560000,image,error);
}
}
