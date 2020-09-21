SELECT type, COUNT(*) AS CountType
FROM Pokemon
GROUP BY type
ORDER BY CountType, type ASC