SELECT type
FROM Pokemon, Evolution
WHERE Pokemon.id = Evolution.before_id
GROUP BY type
HAVING COUNT(*) >= 3
ORDER BY type DESC