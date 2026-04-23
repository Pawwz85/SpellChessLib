#pragma once

/*
	Small utilities for formatting numerical data in tabular format
*/
namespace DataTables {
	template <typename T>
	void initTable(T* table) {};

	template <typename T>
	void clearTable(T* table) {};

	template <typename T>
	const char* getTableLabel(const T* table) { return "Default Table Label"; };

	template <typename T>
	size_t getColumnCount(const T* table) { return 1; }

	template <typename T>
	const char* getColumnLabel(const T* table, const size_t& column_index) { return "<new column>"; };
}

