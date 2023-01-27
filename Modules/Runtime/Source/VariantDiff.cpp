/*
* This file is a portion of Luna SDK.
* For conditions of distribution and use, see the disclaimer
* and license in LICENSE.txt
* 
* @file VariantDiff.cpp
* @author JXMaster
* @date 2022/6/26
*/
#pragma once
#include "../PlatformDefines.hpp"
#define LUNA_RUNTIME_API LUNA_EXPORT
#include "../VariantDiff.hpp"

namespace Luna
{
	//! Longest Common Subsequence, helper class to diff array.
	struct LCSMatrix
	{
		Vector<usize> data;
		usize row_size;

		usize get(usize x, usize y) const { return data[x + y * row_size]; }
		void set(usize x, usize y, usize v) { data[x + y * row_size] = v; }
	};

	struct LCS
	{
		Vector<usize> sequence;
		Vector<usize> indices1;
		Vector<usize> indices2;

	};

	inline usize find_index(const Vector<usize>& vec, usize v)
	{
		for (usize i = 0; i < vec.size(); ++i)
		{
			if (vec[i] == v) return i;
		}
		return USIZE_MAX;
	}

	static LCSMatrix lcs_internal(const Variant& before, const Variant& after, usize before_begin, usize before_size, usize after_begin, usize after_size)
	{
		LCSMatrix result;
		result.row_size = before_size + 1;
		result.data.resize(result.row_size * (after_size + 1), 0);
		for (usize i = 1; i <= before_size; ++i)
		{
			for (usize j = 1; j <= after_size; ++j)
			{
				if (before[before_begin + i - 1] == after[after_begin + j - 1])
				{
					result.set(i, j, result.get(i - 1, j - 1) + 1);
				}
				else
				{
					result.set(i, j, max(result.get(i - 1, j), result.get(i, j - 1)));
				}
			}
		}
		return result;
	}

	static LCS lcs_backtrack(LCSMatrix& matrix, const Variant& before, const Variant& after, usize before_begin, usize before_size, usize after_begin, usize after_size)
	{
		LCS result;
		usize i = 1;
		usize j = 1;
		while (i <= before_size && j <= after_size)
		{
			// If the JSON tokens at the same position are both Objects or both Arrays, we just say they 
			// are the same even if they are not, because we can package smaller deltas than an entire 
			// object or array replacement by doing object to object or array to array diff.
			if (before[before_begin + i - 1] == after[after_begin + j - 1]
				|| (before[before_begin + i - 1].type() == Variant::Type::object && after[after_begin + j - 1].type() == Variant::Type::object)
				|| (before[before_begin + i - 1].type() == Variant::Type::array && after[after_begin + j - 1].type() == Variant::Type::array))
			{
				result.sequence.push_back(before_begin + i - 1);
				result.indices1.push_back(i - 1);
				result.indices2.push_back(j - 1);
				++i;
				++j;
				continue;
			}
			if (matrix.get(i, j - 1) > matrix.get(i - 1, j))
			{
				++i;
			}
			else
			{
				++j;
			}
		}
		return result;
	}

	static LCS lcs_get(const Variant& before, const Variant& after, usize before_begin, usize before_size, usize after_begin, usize after_size)
	{
		LCSMatrix matrix = lcs_internal(before, after, before_begin, before_size, after_begin, after_size);
		return lcs_backtrack(matrix, before, after, before_begin, before_size, after_begin, after_size);
	}

	constexpr u64 VARIANT_DIFF_OP_DELETED = 0;
	constexpr u64 VARIANT_DIFF_OP_ARRAYMOVE = 3;

	Variant diff_object(const Variant& before, const Variant& after);

	Variant diff_array(const Variant& before, const Variant& after);

	LUNA_RUNTIME_API Variant diff_variant(const Variant& before, const Variant& after)
	{
		if (before.type() == Variant::Type::object && after.type() == Variant::Type::object)
		{
			return diff_object(before, after);
		}
		if (before.type() == Variant::Type::array && after.type() == Variant::Type::array)
		{
			return diff_array(before, after);
		}
		// Simply records two values.
		if (before != after)
		{
			Variant diff_patch(Variant::Type::array);
			diff_patch.push_back(before);
			diff_patch.push_back(after);
			return diff_patch;
		}
		// Returns one null value if equal.
		return Variant();
	}

	static Variant diff_object(const Variant& before, const Variant& after)
	{
		Variant diff_patch(Variant::Type::object);
		// Find properties modified or deleted.
		for (auto& lp : before.key_values())
		{
			const Variant& rp = after[lp.first];

			// Property deleted
			if (rp.type() == Variant::Type::null)
			{
				Variant delta(Variant::Type::array);
				delta.push_back(lp.second);
				delta.push_back((u64)0);
				delta.push_back(VARIANT_DIFF_OP_DELETED);
				diff_patch[lp.first] = move(delta);
				continue;
			}

			// Property changed.
			Variant d = diff_variant(lp.second, rp);
			if (d.type() != Variant::Type::null)
			{
				diff_patch[lp.first] = move(d);
			}
		}
		// Find properties that were added.
		for (auto& rp : after.key_values())
		{
			if (before[rp.first].type() != Variant::Type::null) continue;
			Variant v(Variant::Type::array);
			v.push_back(rp.second);
			diff_patch[rp.first] = move(v);
		}
		if (!diff_patch.empty()) return diff_patch;
		return Variant();
	}

	static Variant diff_array(const Variant& before, const Variant& after)
	{
		Variant result(Variant::Type::object);
		result["_t"] = "a";
		usize common_head = 0;
		usize common_tail = 0;
		if (before == after) return Variant();
		// Find common head
		while (common_head < before.size() 
			&& common_head < after.size() 
			&& before[common_head] == after[common_head])
		{
			++common_head;
		}
		// Find common tail
		while (common_tail + common_head < before.size() 
			&& common_tail + common_head < after.size()
			&& before[before.size() - 1 - common_tail] == after[after.size() - 1 - common_tail])
		{
			++common_tail;
		}
		if (common_head + common_tail == before.size())
		{
			// Trivial case, a block (1 or more consecutive items) was added
			for (usize index = common_head; index < after.size() - common_tail; ++index)
			{
				c8 buf[32];
				snprintf(buf, 32, "%llu", (u64)index);
				Variant v(Variant::Type::array);
				v.push_back(after[index]);
				result[buf] = move(v);
			}
			return result;
		}
		if (common_head + common_tail == after.size())
		{
			// Trivial case, a block (1 or more consecutive items) was removed
			for (usize index = common_head; index < before.size() - common_tail; ++index)
			{
				c8 buf[32];
				snprintf(buf, 32, "_%llu", (u64)index);
				Variant v(Variant::Type::array);
				v.push_back(before[index]);
				v.push_back((u64)0);
				v.push_back(VARIANT_DIFF_OP_DELETED);
				result[buf] = move(v);
			}
			return result;
		}

		// Complex Diff, find the LCS (Longest Common Subsequence)
		LCS lcs = lcs_get(before, after, common_head, before.size() - common_tail - common_head, common_head, after.size() - common_tail - common_head);
		for (usize index = common_head; index < before.size() - common_tail; ++index)
		{
			if (find_index(lcs.indices1, index - common_head) == USIZE_MAX)
			{
				// Removed.
				c8 buf[32];
				snprintf(buf, 32, "_%llu", (u64)index);
				Variant v(Variant::Type::array);
				v.push_back(before[index]);
				v.push_back((u64)0);
				v.push_back(VARIANT_DIFF_OP_DELETED);
				result[buf] = move(v);
			}
		}
		for (usize index = common_head; index < after.size() - common_tail; ++index)
		{
			usize index_after = find_index(lcs.indices2, index - common_head);
			if (index_after == USIZE_MAX)
			{
				// Added
				c8 buf[32];
				snprintf(buf, 32, "%llu", (u64)index);
				Variant v(Variant::Type::array);
				v.push_back(after[index]);
				result[buf] = move(v);
			}
			else
			{
				usize bi = lcs.indices1[index_after] + common_head;
				usize ai = lcs.indices2[index_after] + common_head;
				Variant diff = diff_variant(before[bi], after[ai]);
				if (diff.type() != Variant::Type::null)
				{
					c8 buf[32];
					snprintf(buf, 32, "%llu", (u64)index);
					result[buf] = move(diff);
				}
			}
		}
		return result;
	}

	void patch_object(Variant& before, const Variant& patch);
	void patch_array(Variant& before, const Variant& patch);

	LUNA_RUNTIME_API void patch_variant_diff(Variant& before, const Variant& diff)
	{
		if (diff.type() == Variant::Type::object)
		{
			auto& array_magic = diff["_t"];
			if (before.type() == Variant::Type::array 
				&& array_magic.str() == "a")
			{
				patch_array(before, diff);
				return;
			}
			patch_object(before, diff);
			return;
		}
		if (diff.type() == Variant::Type::array)
		{
			if (diff.size() == 1) // Add
			{
				before = diff[0];
				return;
			}
			if (diff.size() == 2) // Replace
			{
				before = diff[1];
				return;
			}
			if (diff.size() == 3) // Delete, Move or TextDiff
			{
				u64 op = diff[2].unum();
				if (op == VARIANT_DIFF_OP_DELETED)
				{
					before = Variant();
					return;
				}
				// TextDiff not implemented.
			}
		}
	}

	void reverse_object(Variant& after, const Variant& patch);
	void reverse_array(Variant& after, const Variant& patch);

	LUNA_RUNTIME_API void reverse_variant_diff(Variant& after, const Variant& diff)
	{
		if (diff.type() == Variant::Type::object)
		{
			auto& array_magic = diff["_t"];
			if (after.type() == Variant::Type::array
				&& array_magic.str() == "a")
			{
				reverse_array(after, diff);
				return;
			}
			reverse_object(after, diff);
			return;
		}
		if (diff.type() == Variant::Type::array)
		{
			if (diff.size() == 1) // Add (we need to remove the property)
			{
				after = Variant();
				return;
			}
			if (diff.size() == 2) // Replace
			{
				after = diff[0];
				return;
			}
			if (diff.size() == 3) // Delete, Move or TextDiff
			{
				u64 op = diff[2].unum();
				if (op == VARIANT_DIFF_OP_DELETED)
				{
					after = diff[0];
					return;
				}
				// TextDiff not implemented.
			}
		}
	}

	static void patch_object(Variant& before, const Variant& patch)
	{
		for (auto& diff : patch.key_values())
		{
			auto& property = before[diff.first];
			auto& patch_value = diff.second;

			// We need to special case deletion when doing objects since a delete is a removal of a property
			// not a null assignment
			if (patch_value.type() == Variant::Type::array && patch_value.size() == 3 && patch_value[2].unum() == VARIANT_DIFF_OP_DELETED)
			{
				before.erase(diff.first);
			}
			else
			{
				patch_variant_diff(property, patch_value);
			}
		}
	}

	static void reverse_object(Variant& after, const Variant& patch)
	{
		for (auto& diff : patch.key_values())
		{
			auto& property = after[diff.first];
			auto& patch_value = diff.second;

			// We need to special case addition when doing objects since an undo add is a removal of a property
			// not a null assignment
			if (patch_value.type() == Variant::Type::array && patch_value.size() == 1)
			{
				after.erase(diff.first);
			}
			else
			{
				reverse_variant_diff(property, patch_value);
			}
		}
	}

	/*
		Array format:
		{
			_t: "a",
			_n: delta,
			n: delta
		}

		n: refers to the index in the final (after) state of the array, this is used to indicate items inserted.
		_n: refers to the index in the original (before) state of the array, this is used to indicate items removed, or moved.
		
		delta:
		n: [new_value] - insert.
		n: [old_value, new_value] - modified.
		n: {...} - modified with inner changes.
		_n: [old_value, 0, 0] - removed.
		_n: ['', new_dest, 3] - moved.
	*/

	static void patch_array(Variant& before, const Variant& patch)
	{
		Vector<usize> to_remove;
		Vector<Pair<usize, Variant>> to_insert;
		Vector<Pair<usize, Variant>> to_modify;

		for (auto& op : patch.key_values())
		{
			if (op.first == "_t")
				continue;
			auto& value = op.second;
			if (op.first.c_str()[0] == '_')
			{
				// removed item from original array
				if (value.type() == Variant::Type::array && value.size() == 3
					&& (value[2].unum() == VARIANT_DIFF_OP_DELETED || value[2].unum() == VARIANT_DIFF_OP_ARRAYMOVE))
				{
					usize remove_index = (usize)atoll(op.first.c_str() + 1);
					to_remove.push_back(remove_index);
					if (value[2].unum() == VARIANT_DIFF_OP_ARRAYMOVE)
					{
						usize insert_index = (usize)value[1].unum();
						Variant v(Variant::Type::array);
						v.push_back(move(before[remove_index]));
						to_insert.push_back(make_pair(insert_index, move(v)));
					}
				}
			}
			else
			{
				usize insert_index = (usize)atoll(op.first.c_str());
				if (value.type() == Variant::Type::array && value.size() == 1)
				{
					to_insert.push_back(make_pair(insert_index, op.second));
				}
				else
				{
					to_modify.push_back(make_pair(insert_index, op.second));
				}
			}
		}

		// remove items, in reverse order to avoid sawing our own floor
		sort(to_remove.begin(), to_remove.end());
		for (auto iter = to_remove.rbegin(); iter != to_remove.rend(); ++iter)
		{
			before.erase(*iter);
		}

		// insert items.
		auto sort_less = [](const Pair<usize, Variant>& lhs, const Pair<usize, Variant>& rhs) {return lhs.first < rhs.first; };
		sort(to_insert.begin(), to_insert.end(), sort_less);
		for (auto& op : to_insert)
		{
			before.insert(op.first, move(op.second.at(0)));
		}

		for (auto& op : to_modify)
		{
			patch_variant_diff(before.at(op.first), op.second);
		}
	}

	static void reverse_array(Variant& after, const Variant& patch)
	{
		Vector<usize> to_remove;
		Vector<Pair<usize, Variant>> to_insert;
		Vector<Pair<usize, Variant>> to_modify;

		for (auto& op : patch.key_values())
		{
			if (op.first == "_t")
				continue;
			auto& value = op.second;
			if (op.first.c_str()[0] == '_')
			{
				// removed item from original array
				if (value.type() == Variant::Type::array && value.size() == 3
					&& (value[2].unum() == VARIANT_DIFF_OP_DELETED || value[2].unum() == VARIANT_DIFF_OP_ARRAYMOVE))
				{
					usize insert_index = (usize)atoll(op.first.c_str() + 1);
					if (value[2].unum() == VARIANT_DIFF_OP_ARRAYMOVE)
					{
						usize remove_index = (usize)value[1].unum();
						Variant v(Variant::Type::array);
						v.push_back(move(after[remove_index]));
						to_insert.push_back(make_pair(insert_index, move(v)));
						to_remove.push_back(remove_index);
					}
					else
					{
						// reverse removal
						Variant v(Variant::Type::array);
						v.push_back(move(value[0]));
						to_insert.push_back(make_pair(insert_index, move(v)));
					}
				}
			}
			else
			{
				usize insert_index = (usize)atoll(op.first.c_str());
				if (value.type() == Variant::Type::array && value.size() == 1)
				{
					// reverse insertion.
					to_remove.push_back(insert_index);
				}
				else
				{
					to_modify.push_back(make_pair(insert_index, op.second));
				}
			}
		}

		// We need to do everything in reverse order of `patch_array`

		// first modify entries
		for (auto& op : to_modify)
		{
			reverse_variant_diff(after.at(op.first), op.second);
		}

		// remove items, in reverse order to avoid sawing our own floor
		sort(to_remove.begin(), to_remove.end());
		for (auto iter = to_remove.rbegin(); iter != to_remove.rend(); ++iter)
		{
			after.erase(*iter);
		}

		// insert items.
		auto sort_less = [](const Pair<usize, Variant>& lhs, const Pair<usize, Variant>& rhs) {return lhs.first < rhs.first; };
		sort(to_insert.begin(), to_insert.end(), sort_less);
		for (auto& op : to_insert)
		{
			after.insert(op.first, move(op.second.at(0)));
		}
	}

	LUNA_RUNTIME_API void variant_diff_prefix(Variant& diff, const Vector<Variant>& prefix_nodes)
	{
		for (auto iter = prefix_nodes.rbegin(); iter != prefix_nodes.rend(); ++iter)
		{
			Variant child = move(diff);
			diff = Variant(Variant::Type::object);
			if (iter->type() == Variant::Type::string)
			{
				// property.
				diff[iter->str()] = move(child);
			}
			else if (iter->type() == Variant::Type::number)
			{
				// Array index.
				diff["_t"] = "a";
				c8 buf[32];
				snprintf(buf, 32, "%lld", iter->unum());
				diff[buf] = move(child);
			}
			else
			{
				lupanic_msg("Bad prefix_nodes ndoe type.");
			}
		}
	}
}