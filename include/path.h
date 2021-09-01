#ifndef URI_H
#define URI_H

#include <buffer.h>
#include <array.h>

/**
 * Parse an absolute path from the input buffer. If the path is parsed successfully,
 * return the number of characters successfully parsed and store a view to the parsed range
 * in the `res` parameter. Return -2 if the path cannot be parsed successfully.
 *
 * @param buffer: the input buffer.
 * @return the number of characters successfully parsed or -2 in case of error.
 */
long parse_absolute_path(buffer_view_t buffer, array_t *segments);

/**
 * Parse a path segment from the input buffer. If the path segment is parsed successfully,
 * return the number of characters successfully parsed and store a view to the parsed range
 * in the `res` parameter. Return -2 if the segment cannot be successfully parsed.
 *
 * @param buffer: the input buffer.
 * @return the number of characters successfully parsed or -2 in case of error.
 */
long parse_path_segment(buffer_view_t buffer);

/**
 *
 * Decode the given percent-encoded uri. If successful, return 0 and store the result in
 * the `res` parameter. If the segment cannot be successfully decided, return -1.
 * If the res buffer is not NULL and the input segment is composed of only
 * pchars (as defined in rfc3986 section 3.3) then this function will always succeed.
 *
 * @param seg: the percent encoded segment.
 * @param res: the resulting decoded segment.
 * @return 0 if successful and -1 otherwise.
 */
int path_segment_decode(buffer_t seg, buffer_t *res);

/**
 * Percent-encode the input segment. If successful, return 0 and store the result in
 * the `res` paramter. If unsuccessful, return -1. If res is not NULL, this function
 * will always succeed.
 *
 * @param seg: the segment
 * @param res: the resulting encoded segment.
 * @return 0 if successful and -1 otherwise.
 */
int path_segment_encode(buffer_t seg, buffer_t *res);

/**
 * Normalize the input path. If successful, return 0 and store the result in `res` paramter.
 * If unsuccessful, return -1. If the input path is a valid relative or absolute path, then
 * is function will always succeed. The input path is normalized by the following steps:
 *
 * 1. Percent-encoded triplets are converted to uppercase.
 * 2. Percent-encoded unreserved characters are decoded.
 * 3. All dot segments are applied using algorithm in rfc3986 section 5.2.4
 *
 * @param path: the input path.
 * @param res: the resulting normalized path.
 * @return 0 if successful and -1 otherwise.
 */
int path_normalize(buffer_t path, buffer_t *res);

#endif /* URI_H */
