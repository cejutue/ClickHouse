#include <iostream>

#include <Poco/SharedPtr.h>

#include <DB/IO/WriteBufferFromOStream.h>
#include <DB/Storages/StorageLog.h>
#include <DB/DataStreams/TabSeparatedRowOutputStream.h>
#include <DB/DataStreams/LimitBlockInputStream.h>
#include <DB/DataStreams/copyData.h>
#include <DB/DataTypes/DataTypesNumberFixed.h>
#include <DB/Columns/ColumnsNumber.h>

using Poco::SharedPtr;


int main(int argc, char ** argv)
{
	try
	{
		const size_t rows = 10000000;

		/// создаём таблицу с парой столбцов
	
		SharedPtr<DB::NamesAndTypes> names_and_types = new DB::NamesAndTypes;
		(*names_and_types)["a"] = new DB::DataTypeUInt64;
		(*names_and_types)["b"] = new DB::DataTypeUInt8;
		
		DB::StorageLog table("./", "test", names_and_types, ".bin");

		/// пишем в неё
		{
			DB::Block block;

			DB::ColumnWithNameAndType column1;
			column1.name = "a";
			column1.type = (*names_and_types)["a"];
			column1.column = column1.type->createColumn();
			DB::ColumnUInt64::Container_t & vec1 = dynamic_cast<DB::ColumnUInt64&>(*column1.column).getData();

			vec1.resize(rows);
			for (size_t i = 0; i < rows; ++i)
				vec1[i] = 'z';

			block.insert(column1);

			DB::ColumnWithNameAndType column2;
			column2.name = "b";
			column2.type = (*names_and_types)["b"];
			column2.column = column2.type->createColumn();
			DB::ColumnUInt8::Container_t & vec2 = dynamic_cast<DB::ColumnUInt8&>(*column2.column).getData();

			vec2.resize(rows);
			for (size_t i = 0; i < rows; ++i)
				vec2[i] = 'x';

			block.insert(column2);

			SharedPtr<DB::IBlockOutputStream> out = table.write(0);
			out->write(block);
		}
	
		/// читаем из неё
		{
			DB::ColumnNames column_names;
			column_names.push_back("a");
			column_names.push_back("b");

			SharedPtr<DB::IBlockInputStream> in = table.read(column_names, 0);

			Poco::SharedPtr<DB::DataTypes> data_types = new DB::DataTypes;
			data_types->push_back(new DB::DataTypeUInt64);
			data_types->push_back(new DB::DataTypeUInt8);

			DB::WriteBufferFromOStream out_buf(std::cout);
			
			DB::LimitBlockInputStream in_limit(in, 10);
			DB::TabSeparatedRowOutputStream output(out_buf, data_types);
			
			DB::copyData(in_limit, output);
		}
	}
	catch (const DB::Exception & e)
	{
		std::cerr << e.what() << ", " << e.message() << std::endl;
		return 1;
	}

	return 0;
}
