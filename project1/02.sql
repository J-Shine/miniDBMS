SELECT p4.name
FROM Pokemon AS p4
WHERE p4.type IN (
  SELECT p3.type
  FROM Pokemon AS p3
  GROUP BY p3.type
  HAVING COUNT(*) >= (
    SELECT MAX(t2.CountType2)
    FROM (
      SELECT p2.type, COUNT(*) AS CountType2
      FROM Pokemon AS p2
      GROUP BY p2.type
      HAVING COUNT(*) <> (
        SELECT MAX(t1.CountType1)
        FROM (
          SELECT p1.type, COUNT(*) AS CountType1
          FROM Pokemon AS p1
          GROUP BY p1.type
        ) AS t1
      )
    ) AS t2
  )
)
ORDER BY p4.name ASC