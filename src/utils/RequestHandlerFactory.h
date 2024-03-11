#include "handlers/CardHandler.h"
#include "handlers/FolderHandler.h"
#include "Types.h"

class RequestHandlerFactory
{
public:
    static std::unique_ptr<IEntityHandler> createHandler(DataType type)
    {
        switch (type)
        {
        case DataType::Card:
            return std::make_unique<CardHandler>();
        case DataType::Folder:
            return std::make_unique<FolderHandler>();
        default:
            throw std::invalid_argument("Unsupported data type");
        }
    }
};
